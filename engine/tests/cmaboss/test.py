import cmaboss
import numpy as np
import pandas as pd
from unittest import TestCase
from sys import platform
import os
import tempfile


def read_probtraj_entropies(filename):
    # the TH, ErrorTH and H columns of a hexfloat probtraj file
    with open(filename) as f:
        lines = f.read().splitlines()
    assert lines[0].split("\t")[1:4] == ["TH", "ErrorTH", "H"]
    return np.array([[float.fromhex(v) for v in line.split("\t")[1:4]] for line in lines[1:]])


def read_probtraj(filename):
    # times, entropies (TH, ErrorTH, H), probabilities and errors of a MaBoSS probtraj file
    with open(filename) as f:
        lines = f.read().splitlines()
    first_state = lines[0].split("\t").index("State")
    times, entropies, probas, errors = [], [], [], []
    for line in lines[1:]:
        values = line.split("\t")
        times.append(float(values[0]))
        entropies.append([float(v) for v in values[1:4]])
        probas.append({values[i]: float(values[i+1]) for i in range(first_state, len(values), 3)})
        errors.append({values[i]: float(values[i+2]) for i in range(first_state, len(values), 3)})
    return (
        times, np.array(entropies),
        pd.DataFrame(probas).fillna(0.0), pd.DataFrame(errors).fillna(0.0)
    )


def read_statdist(filename):
    # parses a hexfloat statdist file into the trajectories table, the cluster tables and the
    # clusters summary, each as a {(number, state): value} dict
    with open(filename) as f:
        blocks = f.read().split("\n\n")

    def parse_rows(rows, step):
        probas, errors = {}, {}
        for row in rows:
            values = row.split("\t")
            number = int(values[0][1:])
            for i in range(1, len(values), step):
                probas[(number, values[i])] = float.fromhex(values[i+1])
                if step == 3:
                    errors[(number, values[i])] = float.fromhex(values[i+2])
        return probas, errors

    statdist, _ = parse_rows(blocks[0].splitlines()[1:], 2)
    clusters = [parse_rows(block.splitlines()[1:], 2)[0] for block in blocks[1:-1]]
    summary = parse_rows(blocks[-1].strip("\n").splitlines()[1:], 3)
    return statdist, clusters, summary


def table_to_dict(data, numbers, states):
    return {
        (number, state): data[i, j]
        for i, number in enumerate(numbers) for j, state in enumerate(states)
        if data[i, j] != 0.0
    }

class TestCMaBoSS(TestCase):

    def test_load_model(self):
        net = cmaboss.MaBoSSNet(network="models/metastasis/metastasis.bnd")
        cfg = cmaboss.MaBoSSCfg(net, "models/metastasis/metastasis.cfg")
        sim = cmaboss.MaBoSSSim(net=net, cfg=cfg)
        res = sim.run()

    def test_load_model_cellcycle(self):
        # same run as the MaBoSS cellcycle 6 threads test, compared with its reference files
        net = cmaboss.MaBoSSNet(network="models/cellcycle/cellcycle.bnd")
        cfg = cmaboss.MaBoSSCfg(net, config_files=["models/cellcycle/cellcycle_runcfg.cfg", "models/cellcycle/cellcycle_runcfg-thread_6.cfg"])
        sim = cmaboss.MaBoSSSim(net=net, cfg=cfg)
        res = sim.run()

        # the reference has 6 decimals for the probabilities, 4 for the entropies
        times, entropies, probas, errors = read_probtraj("models/cellcycle/refer/Cell_cycle_thread_6_probtraj.csv")
        data, timepoints, states, data_errors = res.get_probtraj()
        np.testing.assert_allclose(timepoints, times)
        table = pd.DataFrame(data, columns=states).reindex(columns=probas.columns, fill_value=0.0)
        table_errors = pd.DataFrame(data_errors, columns=states).reindex(columns=errors.columns, fill_value=0.0)
        self.assertEqual(set(states), set(probas.columns))
        np.testing.assert_allclose(table.values, probas.values, rtol=0, atol=5e-7)
        np.testing.assert_allclose(table_errors.values, errors.values, rtol=0, atol=5e-7)

        data, _, _ = res.get_entropy_trajectory()
        np.testing.assert_allclose(data, entropies, rtol=0, atol=5e-5)

        # Cell_cycle_thread_6_fp.csv: "Fixed Points (0)"
        self.assertEqual(res.get_fp_table(), {})

        # Cell_cycle_thread_6_statdist.csv: 5000 trajectories, all <nil>, in a single cluster
        data, trajectories, states = res.get_statdist()
        self.assertEqual(states, ["<nil>"])
        self.assertEqual(trajectories, list(range(1, 5001)))
        np.testing.assert_array_equal(data, np.ones((5000, 1)))
        clusters = res.get_statdist_clusters()
        self.assertEqual(len(clusters), 1)
        self.assertEqual(clusters[0][1], list(range(1, 5001)))
        data, numbers, states, data_errors = res.get_statdist_clusters_summary()
        self.assertEqual((data.tolist(), numbers, states, data_errors.tolist()), ([[1.0]], [1], ["<nil>"], [[0.0]]))

    def test_config_arguments_errors(self):
        net = cmaboss.MaBoSSNet(network="models/cellcycle/cellcycle.bnd")
        main_cfg = "models/cellcycle/cellcycle_runcfg.cfg"
        thread_cfg = "models/cellcycle/cellcycle_runcfg-thread_6.cfg"
        with self.assertRaises(TypeError):
            # the third positional argument is config_files
            cmaboss.MaBoSSCfg(net, main_cfg, thread_cfg)
        with self.assertRaises(TypeError):
            cmaboss.MaBoSSCfg(net, main_cfg, [thread_cfg])
        with self.assertRaises(TypeError):
            cmaboss.MaBoSSCfg(net, config_files=thread_cfg)
        with self.assertRaises(TypeError):
            cmaboss.MaBoSSCfg(net, config_files=[main_cfg, None])
        with self.assertRaises(TypeError):
            cmaboss.MaBoSSCfg(net, config_file=[main_cfg])
        with self.assertRaises(TypeError):
            cmaboss.MaBoSSCfg(net, config_file=main_cfg, config_str="thread_count = 6;")
        with self.assertRaises(TypeError):
            cmaboss.MaBoSSSim(network="models/cellcycle/cellcycle.bnd", config=main_cfg, configs=[thread_cfg])

    def test_set_output(self):
        net = cmaboss.MaBoSSNet(network="models/metastasis/metastasis.bnd")
        outputs = net.get_output()

        for wrong, exception in [
            (["Apoptosis", "NotANode"], cmaboss.BNException),
            (["Apoptosis", 3], TypeError),
            ("Apoptosis", TypeError),
            (3, TypeError),
        ]:
            with self.assertRaises(exception):
                net.set_output(wrong)
            # a rejected list leaves the outputs untouched
            self.assertEqual(net.get_output(), outputs)

        net.set_output(("Apoptosis", "Metastasis"))
        self.assertEqual(net.get_output(), ["Metastasis", "Apoptosis"])

        sim = cmaboss.PopMaBoSSSim("models/popmaboss/Fork.bnd", "models/popmaboss/Fork.cfg")
        with self.assertRaises(cmaboss.BNException):
            sim.network.set_output(["A", "NotANode"])
        sim.network.set_output(["A"])
        self.assertEqual(sim.network.get_output(), ["A"])

    def test_simulate(self):
        sim = cmaboss.MaBoSSSim(network="models/metastasis/metastasis.bnd", config="models/metastasis/metastasis.cfg")
        res = sim.run()
        res.get_probtraj()
        res.get_last_probtraj()
        res.get_nodes_probtraj()
        res.get_last_nodes_probtraj()
        res.get_fp_table()
        
    def test_load_model_error(self):
        with self.assertRaises(cmaboss.BNException):
            cmaboss.MaBoSSSim(network="models/metastasis/metastasis-error.bnd", config="models/metastasis/metastasis.cfg")

    def test_last_states(self):
        expected = {
            '<nil>': 0.206, 'Apoptosis -- CellCycleArrest': 0.426, 'CellCycleArrest': 0.068, 
            'Migration -- Metastasis -- Invasion -- CellCycleArrest': 0.3
        }

        expected_nodes = {
            'Metastasis': 0.3, 'Invasion': 0.3, 'Migration': 0.3, 'Apoptosis': 0.426, 'CellCycleArrest': 0.794
        }

        sim = cmaboss.MaBoSSSim(network="models/metastasis/metastasis.bnd", config="models/metastasis/metastasis.cfg")
        res = sim.run(only_last_state=False)
        raw_res, _, states, _ = res.get_last_probtraj()
        for i, value in enumerate(np.nditer(raw_res)):
            self.assertAlmostEqual(value, expected[states[i]])
        
        raw_nodes_res, _, nodes, _ = res.get_last_nodes_probtraj()
        for i, value in enumerate(np.nditer(raw_nodes_res)):
            self.assertAlmostEqual(value, expected_nodes[nodes[i]])

        simfinal = cmaboss.MaBoSSSim(network="models/metastasis/metastasis.bnd", config="models/metastasis/metastasis.cfg")
        resfinal = simfinal.run(only_last_state=True)
    
        raw_res, _, states, _ = res.get_last_probtraj()
        for i, value in enumerate(np.nditer(raw_res)):
            self.assertAlmostEqual(value, expected[states[i]])
        
        raw_nodes_res, _, nodes, _ = res.get_last_nodes_probtraj()
        for i, value in enumerate(np.nditer(raw_nodes_res)):
            self.assertAlmostEqual(value, expected_nodes[nodes[i]])
           
    def test_load_model_str(self):
        with open("models/metastasis/metastasis.bnd", "r") as bnd, open("models/metastasis/metastasis.cfg", "r") as cfg:    
            sim = cmaboss.MaBoSSSim(network_str=bnd.read(),config_str=cfg.read())
            res = sim.run()
            res.get_probtraj()
            res.get_last_probtraj()
            res.get_nodes_probtraj()
            res.get_last_nodes_probtraj()
            res.get_fp_table()

    def test_load_model_str_error(self):
        with open("models/metastasis/metastasis-error.bnd", "r") as bnd, open("models/metastasis/metastasis.cfg", "r") as cfg:    
            with self.assertRaises(cmaboss.BNException):
                cmaboss.MaBoSSSim(network_str=bnd.read(),config_str=cfg.read())

    def test_load_sbml(self):
        sim = cmaboss.MaBoSSSim("models/sbml/cell_fate.sbml", "models/sbml/cell_fate.cfg")
        res = sim.run()

    def test_load_sbml_error(self):
        with self.assertRaises(cmaboss.BNException):
            sim = cmaboss.MaBoSSSim("models/sbml/BIOMD0000000562_url.xml")
            
    def test_use_sbml_names(self):
        sim = cmaboss.MaBoSSSim("models/sbml/Cohen.sbml")
        nodes = ",".join(sorted([s.split(" ")[1] for s in sim.str_bnd().split("\n") if s.lower().startswith("node")]))
        self.assertEqual(nodes, "S_1,S_10,S_11,S_12,S_13,S_14,S_15,S_16,S_17,S_18,S_19,S_2,S_20,S_21,S_22,S_23,S_24,S_25,S_26,S_27,S_28,S_29,S_3,S_30,S_31,S_32,S_4,S_5,S_6,S_7,S_8,S_9")
        
        sim = cmaboss.MaBoSSSim("models/sbml/Cohen.sbml", use_sbml_names=True)        
        nodes = ",".join(sorted([s.split(" ")[1] for s in sim.str_bnd().split("\n") if s.lower().startswith("node")]))
        self.assertEqual(nodes, "AKT1,AKT2,Apoptosis,CDH1,CDH2,CTNNB1,CellCycleArrest,DKK1,DNAdamage,ECM,EMT,ERK,GF,Invasion,Metastasis,Migration,NICD,SMAD,SNAI1,SNAI2,TGFbeta,TWIST1,VIM,ZEB1,ZEB2,miR200,miR203,miR34,p21,p53,p63,p73")

    def test_popmaboss(self):
        sim = cmaboss.PopMaBoSSSim("models/popmaboss/Fork.bnd", "models/popmaboss/Fork.cfg")
        res = sim.run()

        data, index, columns, _ = res.get_probtraj()
        table = pd.DataFrame(data, index=index, columns=columns)
        table_expected = pd.DataFrame(
            np.array(
                [[1.66255134e-01, 3.40868015e-02, 3.38551043e-02, 8.07009188e-02,
                8.09583182e-02, 1.18692488e-01, 1.18842921e-01, 1.02495633e-01,
                1.02368440e-01, 1.61744241e-01],
                [4.09896097e-04, 1.04891000e-01, 1.04489977e-01, 3.73902360e-02,
                3.75903000e-02, 6.12595657e-03, 6.10061619e-03, 3.14471487e-01,
                3.13558480e-01, 7.49720510e-02],
                [0.00000000e+00, 1.22262280e-01, 1.21886609e-01, 5.70676378e-03,
                5.81079215e-03, 1.22937771e-04, 1.19760919e-04, 3.66654370e-01,
                3.65839924e-01, 1.15965617e-02],
                [0.00000000e+00, 1.24778404e-01, 1.24344195e-01, 7.40079145e-04,
                8.37198592e-04, 4.63800071e-06, 2.43825933e-06, 3.74330132e-01,
                3.73385687e-01, 1.57722799e-03],
                [0.00000000e+00, 1.25099041e-01, 1.24732671e-01, 7.96508015e-05,
                1.32926015e-04, 0.00000000e+00, 0.00000000e+00, 3.75337963e-01,
                3.74386572e-01, 2.31175526e-04],
                [0.00000000e+00, 1.25146088e-01, 1.24776072e-01, 3.91228414e-06,
                3.25290659e-05, 0.00000000e+00, 0.00000000e+00, 3.75510193e-01,
                3.74499678e-01, 3.15281934e-05],
                [0.00000000e+00, 1.25150000e-01, 1.24782886e-01, 0.00000000e+00,
                1.71140772e-05, 0.00000000e+00, 0.00000000e+00, 3.75532358e-01,
                3.74509794e-01, 7.84793118e-06],
                [0.00000000e+00, 1.25150000e-01, 1.24790000e-01, 0.00000000e+00,
                8.54296115e-07, 0.00000000e+00, 0.00000000e+00, 3.75549146e-01,
                3.74510000e-01, 0.00000000e+00],
                [0.00000000e+00, 1.25150000e-01, 1.24790000e-01, 0.00000000e+00,
                0.00000000e+00, 0.00000000e+00, 0.00000000e+00, 3.75550000e-01,
                3.74510000e-01, 0.00000000e+00],
                [0.00000000e+00, 1.25150000e-01, 1.24790000e-01, 0.00000000e+00,
                0.00000000e+00, 0.00000000e+00, 0.00000000e+00, 3.75550000e-01,
                3.74510000e-01, 0.00000000e+00]]
                , dtype=np.float64
            ),
            np.array([0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0]),
            np.array([
                '[{A:3}]', '[{A -- C:3}]', '[{A -- B:3}]', '[{A:1},{A -- C:2}]', '[{A:1},{A -- B:2}]', '[{A:2},{A -- C:1}]', 
                '[{A:2},{A -- B:1}]', '[{A -- C:1},{A -- B:2}]', '[{A -- C:2},{A -- B:1}]', '[{A:1},{A -- C:1},{A -- B:1}]'
            ])
            
        )
        
        if platform != "darwin":
            pd.testing.assert_frame_equal(table.sort_index(axis=1), table_expected.sort_index(axis=1), check_exact=False, rtol=2e-2, atol=1e-4)

    def test_entropy_trajectory(self):
        sim = cmaboss.MaBoSSSim(network="models/metastasis/metastasis.bnd", config="models/metastasis/metastasis.cfg")
        res = sim.run()
        data, timepoints, columns = res.get_entropy_trajectory()
        _, probtraj_timepoints, _, _ = res.get_probtraj()

        self.assertEqual(columns, ["TH", "ErrorTH", "H"])
        self.assertEqual(timepoints, probtraj_timepoints)

        with tempfile.TemporaryDirectory() as tmpdir:
            res.display_probtraj(os.path.join(tmpdir, "probtraj.csv"), 1)
            expected = read_probtraj_entropies(os.path.join(tmpdir, "probtraj.csv"))
        np.testing.assert_array_equal(data, expected)

    def test_popmaboss_entropy_trajectory(self):
        sim = cmaboss.PopMaBoSSSim("models/popmaboss/Fork.bnd", "models/popmaboss/Fork.cfg")
        res = sim.run()
        data, timepoints, columns = res.get_entropy_trajectory()
        self.assertEqual(columns, ["TH", "ErrorTH", "H"])
        self.assertEqual(len(timepoints), data.shape[0])

        with tempfile.TemporaryDirectory() as tmpdir:
            res.display_probtraj(os.path.join(tmpdir, "probtraj.csv"), os.path.join(tmpdir, "simple_probtraj.csv"), 1)
            expected = read_probtraj_entropies(os.path.join(tmpdir, "probtraj.csv"))
        np.testing.assert_array_equal(data, expected)

    def test_statdist(self):
        # the stationary distributions only cover the reference nodes, so this copy of
        # ToyMod4FP declares A, B and T as such to get two clusters of distinct states
        sim = cmaboss.MaBoSSSim(network="models/toymod4fp/ToyMod4FP.bnd", config="models/toymod4fp/ToyMod4FP.cfg")
        res = sim.run()

        with tempfile.TemporaryDirectory() as tmpdir:
            res.display_statdist(os.path.join(tmpdir, "statdist.csv"), 1)
            expected_statdist, expected_clusters, (expected_summary, expected_errors) = read_statdist(os.path.join(tmpdir, "statdist.csv"))

        # statdist_traj_count is capped by sample_count
        data, trajectories, states = res.get_statdist()
        self.assertEqual(data.shape, (100, len(states)))
        self.assertEqual(trajectories, list(range(1, 101)))
        np.testing.assert_allclose(data.sum(axis=1), 1.0)
        self.assertEqual(table_to_dict(data, trajectories, states), expected_statdist)

        clusters = res.get_statdist_clusters()
        self.assertEqual(len(clusters), 2)
        self.assertEqual(len(expected_clusters), 2)
        self.assertEqual(sum(len(trajs) for _, trajs, _ in clusters), 100)
        for (data, trajectories, states), expected in zip(clusters, expected_clusters):
            self.assertEqual(table_to_dict(data, trajectories, states), expected)

        data, numbers, states, errors = res.get_statdist_clusters_summary()
        self.assertEqual(numbers, [1, 2])
        self.assertTrue((errors > 0).any())
        self.assertEqual(table_to_dict(data, numbers, states), expected_summary)
        self.assertEqual(
            table_to_dict(errors, numbers, states),
            {key: value for key, value in expected_errors.items() if value != 0.0}
        )

        # cached: the clusters are not recomputed
        self.assertIs(res.get_statdist_clusters(), clusters)

#!/bin/sh
#
# test1.sh
#

. `dirname $0`/share.sh

return_code=0

if [ ! -x $POPMABOSS ]
then
    echo $POPMABOSS not found
    exit 1
fi

check_file()
{
    if [ $? = 0 ]; then
	echo "File $1 OK"
    else
	echo "File $1 ** error: differences found **"
    return_code=1
    fi
}

echo
echo "PopMaBoSS test"
rm -rf tmp; mkdir -p tmp
$LAUNCHER $POPMABOSS -c popmaboss/Fork.cfg -o tmp/res_fork popmaboss/Fork.bnd > /dev/null

if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py popmaboss/refer/res_fork_pop_probtraj.csv tmp/res_fork_pop_probtraj.csv 1e-2 1e-4
check_file "pop_projtraj"


$LAUNCHER $POPMABOSS -c popmaboss/Fork.pcfg -o tmp/res_fork popmaboss/Fork.bnd > /dev/null

if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py popmaboss/refer/res_fork_pop_probtraj.csv tmp/res_fork_pop_probtraj.csv 1e-2 1e-4
check_file "pop_projtraj"


$LAUNCHER $POPMABOSS -c popmaboss/Log_Growth.cfg -o tmp/res_log_growth popmaboss/Log_Growth.pbnd >/dev/null

if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py popmaboss/refer/res_log_growth_pop_probtraj.csv tmp/res_log_growth_pop_probtraj.csv --exact
check_file "pop_projtraj"


$LAUNCHER $POPMABOSS -c popmaboss/Assymetric.cfg -o tmp/res_assymetric popmaboss/Assymetric.pbnd > /dev/null

if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py popmaboss/refer/res_assymetric_pop_probtraj.csv tmp/res_assymetric_pop_probtraj.csv 5e-2 5e-2
check_file "pop_projtraj"


$LAUNCHER $POPMABOSS -c popmaboss/ICD_phenomenological_TDC_ratio.cfg -o tmp/res_icd popmaboss/ICD_phenomenologicalPM.pbnd > /dev/null

if [ $? != 0 ]; then exit 1; fi

python compare_probtrajs.py popmaboss/refer/res_icd_custom_pop_probtraj.csv tmp/res_icd_custom_pop_probtraj.csv 5e-2 5e-2
check_file "custom pop_projtraj"

rm -rf tmp; 

exit $return_code

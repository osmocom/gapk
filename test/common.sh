# directory containing the reference files for comparing against
REFDIR=./ref-files

if [ -f ../src/gapk ]; then
	GAPK=../src/gapk
elif [ -f `which gapk` ]; then
	GAPK=`whiich gapk`
else
	exit 1
fi
echo Using gapk found at $GAPK

FORMATS="amr-efr gsm racal-hr racal-fr racal-efr ti-hr ti-fr ti-efr rtp-efr rtp-hr-etsi rtp-hr-ietf"

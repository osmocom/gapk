# directory containing the reference files for comparing against
REFDIR=./ref-files

if [ -f ../src/osmo-gapk ]; then
	GAPK=../src/osmo-gapk
elif [ -f `which osmo-gapk` ]; then
	GAPK=`which osmo-gapk`
else
	exit 1
fi
echo Using osmo-gapk found at $GAPK

FORMATS="amr-efr gsm racal-hr racal-fr racal-efr ti-hr ti-fr ti-efr rtp-efr rtp-hr-etsi rtp-hr-ietf"

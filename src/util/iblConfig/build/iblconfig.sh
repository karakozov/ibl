#!/bin/bash

if [ ${INSYS_BOARD} == INSYS_UNKNOWN ] ;
then
    echo "You need specify INSYS_BOARD environment variable to select board configuration!"
    exit -1;
fi

if [ ${INSYS_BOARD} == "INSYS_PEX_SRIO" ] ;
then
    FNAME_PREFIX="pex_srio"
fi

if [ ${INSYS_BOARD} == "INSYS_DSP6678PEX" ] ;
then
    FNAME_PREFIX="dsp6678pex"
fi

if [ ${INSYS_BOARD} == "INSYS_AC_DSP" ] ;
then
    FNAME_PREFIX="ac_dsp"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC110P" ] ;
then
    FNAME_PREFIX="fmc110p"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC111P" ] ;
then
    FNAME_PREFIX="fmc111p"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC112CP" ] ;
then
    FNAME_PREFIX="fmc112cp"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC112CP_V11" ] ;
then
    FNAME_PREFIX="fmc112cp_v11"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC114V" ] ;
then
    FNAME_PREFIX="fmc114v"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC116V" ] ;
then
    FNAME_PREFIX="fmc116v"
fi

if [ ${INSYS_BOARD} == "INSYS_FMC117CP" ] ;
then
    FNAME_PREFIX="fmc117cp"
fi

if [ ${INSYS_BOARD} == "INSYS_FM408C_1G" ] ;
then
    FNAME_PREFIX="fm408c1g"
fi

if [ ${INSYS_BOARD} == "INSYS_FM408C_2G" ] ;
then
    FNAME_PREFIX="fm408c2g"
fi

if [ ${INSYS_BOARD} == "INSYS_ABC" ] ;
then
    FNAME_PREFIX="abc"
fi

#
# Copy settings to file
#
cp input.cfg input.txt

if [ ${INSYS_CPU} == "CPU0" ] ;
then
    echo "ethBoot-ipAddr = 192.168.0.197" >> input.txt
    CPUNAME_PREFIX="cpu0"
fi

if [ ${INSYS_CPU} == "CPU1" ] ;
then
    echo "ethBoot-ipAddr = 192.168.0.198" >> input.txt
    CPUNAME_PREFIX="cpu1"
fi

echo "ethBoot-fileName = c6678_"${FNAME_PREFIX}_${CPUNAME_PREFIX}.bin >> input.txt

./iblConfig.out input.txt

echo "-------- Build for BOARD: ${INSYS_BOARD} --- CPU: ${INSYS_CPU} --- I2C : ${I2C_BUS_ADDRESS} --------"

cp ibl.bin ibl_c6678_${FNAME_PREFIX}_${CPUNAME_PREFIX}_${I2C_BUS_ADDRESS}.bin;
cp ibl_c6678_${FNAME_PREFIX}_${CPUNAME_PREFIX}_${I2C_BUS_ADDRESS}.bin ../../../../bin/

cp ibl.bin i2crom_${I2C_BUS_ADDRESS}_c6678_le.bin
cp i2crom_${I2C_BUS_ADDRESS}_c6678_le.bin ../../../../../../product/

cd ../../../../../../program_evm/binaries/evm6678l
BUS_ADDRESS=`echo ${I2C_BUS_ADDRESS} | sed -r 's/^.{2}//'`
ln -svf ../../../product/i2crom_${I2C_BUS_ADDRESS}_c6678_le.bin eeprom${BUS_ADDRESS}.bin

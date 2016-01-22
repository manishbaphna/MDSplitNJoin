
cd ..
make all
cd -
 ../serpgm.exe input.csv
 ../despgm.exe ZTM3.bin ZNM3.bin ZFM3.bin ZBM3.bin UBM3.bin
 diff input.csv output.csv -b

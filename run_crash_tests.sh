#!/bin/sh

echo
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo "            WARNING               "
echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
echo
echo "These tests can crash your kernel!"
echo "Only run if you have properly patched kernels!"
echo "Be sure to sync your disk first too."
echo
echo "Waiting 5 seconds before running to give you "
echo "time to change your mind..."
echo
sleep 5

./crashes/pe_task_schedule_panic
./crashes/pe_inherit_memleak
./crashes/oflo_sw_cpu_clock_crash
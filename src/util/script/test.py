#!/usr/bin/python

import os
import time

i = 0

while 1:
	os.system("sudo rm -rf /tmp/sync/* ")
	os.system("sudo /home/getmoon/sync/vmsync/src/util/finit/finit /opt/sync/data/master/testfile0")
	os.system("sudo /home/getmoon/sync/vmsync/src/util/finit/finit /opt/sync/data/slave/testfile0")
	os.system("sudo /home/getmoon/sync/vmsync/src/util/tcase2/tcase /opt/sync/data/master/testfile0 0 4096 sorted 1")

	time.sleep(30);

	a = os.popen("md5sum /opt/sync/data/master/testfile0")
	b = os.popen("md5sum /opt/sync/data/slave/testfile0")
	aa = a.read()
	bb = b.read()
	ra = aa.split()[0]
	rb = bb.split()[0]
	i = i + 1
	print ra
	print rb

	if ra == rb:
		print "runing one times"
	else:
		print "not equal now"
		os.system("sudo /home/getmoon/sync/vmsync/src/util/fverify/fverify /opt/sync/data/slave/testfile0")
		os.exit(0)

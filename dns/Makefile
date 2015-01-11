all: mdask mdwatch test1035

mdask:
	gcc -g -o mdask mdask.c mprint.c 1035.c ichat.c ../misc/dump.c ../misc/b64.c -I../misc ../misc/xht.c

mdwatch:
	gcc -g -o mdwatch mdwatch.c mprint.c 1035.c ichat.c sdtxt.c ../misc/dump.c ../misc/b64.c -I../misc ../misc/xht.c

test1035:

ichat_test: ichat_test.c ichat.c ichat.h
	gcc -g -o ichat_test ichat_test.c ichat.c ../misc/xht.c ../misc/b64.c ../misc/dump.c 1035.c -I../misc

test_push: test_push.c
	gcc -o test_push test_push.c 1035.c ../misc/dump.c ../misc/b64.c ../misc/xht.c ichat.c  -I../misc

mdnsd_test: mdnsd_test.c mdnsd.c mdnsd.h
	gcc -g -o mdnsd_test mdnsd_test.c mdnsd.c mprint.c 1035.c ../misc/dump.c ../misc/b64.c ../misc/xht.c ichat.c  -I../misc

clean:
	rm -f mdask mdwatch test1035 ichat_test mdnsd_test

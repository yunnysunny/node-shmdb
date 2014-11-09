#include <stdio.h>
#include <windows.h>
#include "../include/mm.h"

int main() {
	int ret = 0;

	STARTUPINFO si={sizeof(si)};
	PROCESS_INFORMATION pi;
	GetStartupInfo(&si);

	STHashShareHandle handle;
	int rv = shmdb_initParent(&handle,60);
	if (rv != 0) {
		return rv;
	}
	//printf("Created shared memory status:\n");
	const char *key = "key";
	unsigned short keyLen = (unsigned short)(strlen(key));
	const char *value = "value";
	unsigned short valueLen = (unsigned short)(strlen(value));
	shmdb_getInfo(&handle,NULL);
	rv = shmdb_put(&handle,key,keyLen,value,valueLen);
	if (rv != 0) {
		printf("shmdb_put error:0x%x\n",rv);
		return rv;
	}
	char *getValue = NULL;
	unsigned short getValueLen = 0;
	shmdb_dump(&handle,"d:\\temp\\dump1.dp");
	rv = shmdb_get(&handle,key,keyLen,&getValue,&getValueLen);
	if (rv != 0) {
		printf("shmdb_get error:0x%x\n",rv);
		return rv;
	}
	if (getValue != NULL) {
		printf("the value is %s\n",getValue);
		free(getValue);
		getValue = NULL;
		shmdb_dump(&handle,"d:\\temp\\dump2.dp");
	} else {
							
	}

	char sConLin[256] = {0};
	sprintf_s(sConLin,"win32_test_sub.exe %d %d",(int)(handle.semid),(int)(handle.shmid));

	ret = CreateProcess(  
        NULL,   //  ָ��һ��NULL��β�ġ�����ָ����ִ��ģ��Ŀ��ֽ��ַ���  
        sConLin, // �������ַ���  
        NULL, //    ָ��һ��SECURITY_ATTRIBUTES�ṹ�壬����ṹ������Ƿ񷵻صľ�����Ա��ӽ��̼̳С�  
        NULL, //    ���lpProcessAttributes����Ϊ�գ�NULL������ô������ܱ��̳С�<ͬ��>  
        TRUE,//    ָʾ�½����Ƿ�ӵ��ý��̴��̳��˾����   
        0,  //  ָ�����ӵġ���������������ͽ��̵Ĵ����ı�  
            //  CREATE_NEW_CONSOLE  �¿���̨���ӽ���  
            //  CREATE_SUSPENDED    �ӽ��̴��������ֱ������ResumeThread����  
        NULL, //    ָ��һ���½��̵Ļ����顣����˲���Ϊ�գ��½���ʹ�õ��ý��̵Ļ���  
        NULL, //    ָ���ӽ��̵Ĺ���·��  
        &si, // �����½��̵������������ʾ��STARTUPINFO�ṹ��  
        &pi  // �����½��̵�ʶ����Ϣ��PROCESS_INFORMATION�ṹ��  
        );
	const char *key2 = "xx";
	unsigned short key2Len = (unsigned short)(strlen(key2));
	for(int i=0;i<5;i++) {
		rv = shmdb_get(&handle,key2,key2Len,&getValue,&getValueLen);
		if (rv == 0) {
			printf("%dth get xx,value is %s\n",i,getValue);
			free(getValue);
		} else {
			printf("%dth get failed:0x%x\n",rv);
		}
		Sleep(1000);
	}
	WaitForSingleObject( pi.hProcess, INFINITE );  /*Wait until child process exit*/
	if (ret != 0) {
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);                          /*Close handle*/
	}
	getchar();
	return 0;
}
#include <Python.h>
#include <common/credentials.h>
#include "proto/security.pb.h"
#include <string.h>
int
_fib(int n)
{
    if (n < 2)
        return n;
    else
        return _fib(n-1) + _fib(n-2);
}

static PyObject*
fib(PyObject* self, PyObject* args)
{
    int n;

    if (!PyArg_ParseTuple(args, "i", &n))
        return NULL;

    return Py_BuildValue("i", _fib(n));
}

static PyObject*
add(PyObject* self, PyObject* args)
{
    int n, m;

    if (!PyArg_ParseTuple(args, "ii", &n, &m))
        return NULL;

    return Py_BuildValue("ii", n+m, n-m);
}


static PyObject*
GetTicketAndKeyForClusterInternal(PyObject* self, PyObject* args)
{
    char* clusterName;
	int keyType;
    if (!PyArg_ParseTuple(args, "si", &clusterName, &keyType))
        return NULL;

    printf("Hello =%s %d \n", clusterName, keyType);


    int err = 0;

    mapr::fs::TicketAndKey ticketAndKey;
    mapr::fs::Security *security = mapr::fs::Security::GetSecurityInstance();
    printf("Hello =%s!\n", clusterName);

    err = security->GetTicketAndKeyForCluster((mapr::fs::ServerKeyType) keyType, clusterName, &ticketAndKey);
    printf("Hello error =%d!\n", err);
    int bufSize = ticketAndKey.ByteSize();
	printf("Size = %d\n", bufSize); 
    uint8_t *serializedBuf = new uint8_t[bufSize];

    err = ticketAndKey.SerializeToArray(serializedBuf, bufSize);
    printf("Hello error =%d!\n", err);
    for (int i = 0; i < bufSize; i++)
        printf("%d ", serializedBuf[i]);
    printf("\n");

    return Py_BuildValue("s#", serializedBuf, bufSize);
}

static PyObject*
GenerateRandomNumber(PyObject* self, PyObject* args)
{
    mapr::fs::Security *security = mapr::fs::Security::GetSecurityInstance();
    return Py_BuildValue("l", security->GenerateRandomNumber());
}

static PyObject*
Decrypt(PyObject* self, PyObject* args)
{
 uint8_t* keyBase;
 uint8_t* data;
 int dataLen;
 int keyLen;

 if (!PyArg_ParseTuple(args, "s#s#", &keyBase, &keyLen, &data, &dataLen))
        return NULL;
 
printf("data len = %d; key len = %d\n", dataLen, keyLen);
    mapr::fs::Security *security = mapr::fs::Security::GetSecurityInstance();

  int maxDecryptedSize = security->GetDecryptedSize(dataLen);

  uint8_t *decryptedData = new uint8_t[maxDecryptedSize];
  int decryptedDataLen = maxDecryptedSize;

    int err = security->Decrypt(keyBase, keyLen, data, dataLen, decryptedData, decryptedDataLen, &decryptedDataLen);

      assert(err || (decryptedDataLen == maxDecryptedSize));

    return Py_BuildValue("s#", decryptedData, decryptedDataLen);
}


static PyObject*
Encrypt(PyObject* self, PyObject* args)
{
 uint8_t* keyBase;
 uint8_t* data;
 int dataLen;
 int keyLen;

 if (!PyArg_ParseTuple(args, "s#s#", &keyBase, &keyLen, &data, &dataLen))
        return NULL;

printf("data len = %d; key len = %d\n", dataLen, keyLen);
    mapr::fs::Security *security = mapr::fs::Security::GetSecurityInstance();

  int maxEncryptedSize = security->GetEncryptedSize(dataLen);

  uint8_t *encryptedData = new uint8_t[maxEncryptedSize];
  int encryptedDataLen = maxEncryptedSize;

    int err = security->Encrypt(keyBase, keyLen, data, dataLen, encryptedData, encryptedDataLen, &encryptedDataLen);

      assert(err || (encryptedDataLen == maxEncryptedSize));

    return Py_BuildValue("s#", encryptedData, encryptedDataLen);
}



static PyMethodDef SecurityMethods[] = {
    {"fib", fib, METH_VARARGS, "Calculate the Fibonacci numbers."},
    {"add", add, METH_VARARGS, "ADD."},
    {"GetTicketAndKeyForClusterInternal", GetTicketAndKeyForClusterInternal, METH_VARARGS, "SECURITY."},
    {"GenerateRandomNumber", GenerateRandomNumber, METH_VARARGS, "SECURITY."},
    {"Encrypt", Encrypt, METH_VARARGS, "SECURITY."},
    {"Decrypt", Decrypt, METH_VARARGS, "SECURITY."},
    {NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC
initsecurity(void)
{
    Py_InitModule("security", SecurityMethods);
}

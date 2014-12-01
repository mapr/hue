/* Copyright (c) 2009 & onwards. MapR Tech, Inc., All rights reserved */

#ifndef COMMON_CREDENTIALS_H__
#define COMMON_CREDENTIALS_H__
#include <sys/types.h>
#include <cryptopp/osrng.h>
#include "common/nonlinuxsupport.h"

#ifndef __WINDOWS__
#include <sys/uio.h>
#include <unistd.h>
#endif

#include "proto/security.pb.h"
#include "common/common.h"
#include "common/blacklistedae.h"
#include "rpc/dispatch.h"
#include "cryptopp/sha.h"
#include <string.h>

namespace mapr {
namespace fs {

/* 
 * Default gid of the cluster administrator. This can be over-ridden using
 * the CLDB configuration file. When this is over-ridden, the CLDB will
 * update this gid as part of the HB response.
 */
const int MAPR_CLUSTER_ADMIN_GID = 0;

/*
 * UID used for mapfs->mapfs communication.
 */
const int MAPR_CLUSTER_ADMIN_UID = 0;

// AES -256 key size is 256 bits
const int KeySizeInBytes = 32;


// 16 byte hash to authenticate the data
const int TagSizeInBytes = 16;

bool ValidEncryptedData(const uint8_t *iv, uint32_t size);
#ifdef VALIDATE_ENCRYPTED_DATA 
const int  EncryptedDataSignature = 0xaabbccdd;
const int EncryptedDataSignatureSize = sizeof(int);
#else
const int EncryptedDataSignatureSize = 0;
#endif

const int IVSizeInBytes = 16;
const int EncryptionOverHeadBytes = (EncryptedDataSignatureSize + IVSizeInBytes + TagSizeInBytes);

struct ValidateBlacklistWA {
  WADebugInfo dbgInfo;
  union {
    WADebugInfo childDbgInfo;
    GlobalDispatchWA globWA;
  };

//  Ticket *ticket;
  int srcCpuQid;
  int len;
  const uint8_t  *encryptedTicket;
  // lock and condition for the synchronous part.
  pthread_mutex_t mtx;
  pthread_cond_t cond;
  bool wakeup;

  int err;
  ValidateBlacklistWA *next;
};

class Security;
extern pthread_mutex_t security_mtx;

class ThreadSpecificSecurityInfo {
public:
  CryptoPP::AutoSeededRandomPool rng_;
  uint8_t cachedIV_[IVSizeInBytes];
  ThreadSpecificSecurityInfo() {
    rng_.GenerateBlock(cachedIV_, IVSizeInBytes);

  }
};

class KeyStore {
  struct KeyInfo {
    Key key;
    uint64_t expiryTime;
  };

public:
  KeyStore(ServerKeyType maxKeyType) {
    maxKeyType_ = maxKeyType;
    keyInfo_ = new KeyInfo * [maxKeyType_];
    for (int i = 0; i < maxKeyType_; ++i) {
      keyInfo_[i] = NULL;
    }  
  }
  ~KeyStore() {
    delete [] keyInfo_;
  }

  int SetKey(ServerKeyType keyType, const Key *key) {
    if (keyType > maxKeyType_) {
      return EINVAL;
    }

    KeyInfo *k = new KeyInfo();
    k->key.CopyFrom(*key);

    if (keyInfo_[keyType] != NULL) {
      delete keyInfo_[keyType];
      keyInfo_[keyType] = NULL;
    }

    keyInfo_[keyType] = k;
    return 0;
  }

  int GetKey(ServerKeyType keyType, Key *key) {
    if (keyType > maxKeyType_) {
      return EINVAL;
    }
    if (keyInfo_[keyType] == NULL) {
      return ENOENT;
    }
    key->CopyFrom(keyInfo_[keyType]->key);
    return 0;
  }

  int SetKeyFile(mapr::fs::Security *security,
        ServerKeyType keyType, const char *fileName);
private:
  ServerKeyType maxKeyType_;
  KeyInfo **keyInfo_;
};


class TicketAndKeyStore {
  struct TicketAndKeyInfo {
    char *clusterName;
    TicketAndKey ticketAndKey;
    TicketAndKeyInfo *next;
  };
private:
  ServerKeyType maxKeyType_;
  TicketAndKeyInfo **ticketAndKeyInfo_;

public:
  TicketAndKeyStore(ServerKeyType maxKeyType) {
    maxKeyType_ = maxKeyType;
    ticketAndKeyInfo_ = new TicketAndKeyInfo * [maxKeyType_];
    for (int i = 0; i < maxKeyType_; ++i) {
      ticketAndKeyInfo_[i] = NULL;
    }
  }

  ~TicketAndKeyStore() {
    delete[] ticketAndKeyInfo_;
  }

  int SetTicketAndKey(ServerKeyType keyType, const char *clusterName,
                      const TicketAndKey *ticketAndKey);

  int GetTicketAndKeyForCluster(
    ServerKeyType keyType,
    const char *clusterName, TicketAndKey *ticketAndKey);

   int GetTicketAndKeyEntry(
      ServerKeyType keyType,
      int entryIndex,
      char *outClusterName,
      int outClusterNameLen,
      TicketAndKey *ticketAndKey);

  int AddNewMaprServerTicketEntries(Security *security,
                                    const char *ticketAndKeyFile);

  int SetTicketAndKeyFile(Security *security, const char *fname);

  int SetTicketAndKeyFile(Security *security,
    const char *fname,
    bool newClusterAdded);
};

enum ClusterConfKeyType {
  SecurityKey = 0,
  CldbHttpsPortKey,
  UseKerberoseKey,
  DisableReplayDetectionKey,
  CldbPrincipalKey,
  OpMaxKeyType,  // This has to be the last entry
};

enum ClusterConfValueType {
  boolType = 0, 
  intType,
  strType,
  InvalidValueType,  // Must be last entry
};

struct ClusterConfEntryType {
  ClusterConfKeyType key;
  ClusterConfValueType type;
  char name[32];
  char defaultValue[32];
  bool canOverRide;
};

/* secure : boolean type (true / false)
cldbHttpsPort : integer type
kerberosEnable : boolean type (true / false) 
cldbPrincipal="string possibly containing '/', '-', '@' , '.' , '_' "
*/

const ClusterConfEntryType ClusterOptionTable[] = {
  {SecurityKey, boolType, "secure", "false", false},
  {CldbHttpsPortKey, intType, "cldbHttpsPort", "7443", false},
  {UseKerberoseKey, boolType, "kerberosEnable", "false", false},
  {DisableReplayDetectionKey, boolType, "disableReplayDetection", "false", false},
  {CldbPrincipalKey, strType, "cldbPrincipal", "", false},
  {OpMaxKeyType, InvalidValueType, "", "", false},
};

class MaprClusterOptions {
  struct ClusterOptionInfo {
    ClusterConfKeyType key;
    char *clusterName;
    char *value; 
    ClusterOptionInfo *next;
  };

public:
  MaprClusterOptions() {
    isParsingDone_ = false;
    optionInfo_ = new ClusterOptionInfo * [OpMaxKeyType];
    for(int i = 0; i < OpMaxKeyType; ++i) {
      optionInfo_[i] = NULL;
    }
  }

  ~MaprClusterOptions() {
    delete[] optionInfo_;
  }

  bool IsParsingDone() { 
    return isParsingDone_; 
  }

  void SetParsingDone() {
    isParsingDone_ = true;
  }

  int SetClusterOption(const char *clusterName,
                       const char *key, const char *value);

  int GetClusterOption(const char *clusterName,
                       const char *key, char *value, int n);

  bool IsSecurityEnabled(const char * clusterName);
  bool IsUseKerberoseEnabled(const char * clusterName);
  bool IsReplayDetectionDisabled(const char * clusterName);
  int GetCldbHttpsPort(const char * clusterName);
  int GetCldbPrincipal(const char * clusterName, char *value, int n);
private:
  // return value for bool and int types. return -1 if invalid
  int GetIntValue(const char *clusterName, int key);
  // return pointer to the record if in database, otherwise return NULL
  ClusterOptionInfo* GetRecord(const char *clusterName, int key);
  // return converted int value. return -1 if invalid
  int ConvertValueToInt(int key, const char *value);
  bool IsValidValue(int key, const char *value);
  
  ClusterOptionInfo **optionInfo_;
  bool isParsingDone_;
  static const int MAX_PORT_NO = 65535; 
};

// Returns 0 if OK, otherwise returns Error Code
// Parse Cluster Options (key=value), e.g., security=ON
int ParseClusterOptions(const char *clusterName, char *option);

// Mark Parsing Done for Cluster Conf (mapr-clusters.conf)
// Required to check any error where we call Get before parsing
void SetClusterConfParsingDone(void);

class Security {

public:
  static const uint64_t MAX_EXPIRY_TIME = LLONG_MAX / 10000;

  static bool IsSecurityEnabled(const char *clusterName) {
    Security *security = GetSecurityInstance();
    return security->maprClusterOptions_.IsSecurityEnabled(clusterName);
  }
  
  bool DataSecurityOn(const char *clusterName) {
    return maprClusterOptions_.IsSecurityEnabled(clusterName);
  }  

  inline void AddCredentials(CredentialsMsg *c) {
    c->CopyFrom(creds_);
  }

  // Fill user credentials in the c. 
  inline void AddUserCredentials(CredentialsMsg *c) {
    c->set_uid(uid_);
    c->add_gids(egid_);

    for(int i = 0; i < numgroups_; ++i) {
      if (gids_[i] == egid_) continue;
        c->add_gids(gids_[i]);
    }
  }

  inline void AddRootCredentials(CredentialsMsg *c) {
    c->set_uid(MAPR_CLUSTER_ADMIN_UID);
    c->add_gids(adminGid_);
  }

  void SetAdminGid(int gid);
  static Security *GetSecurityInstance();

  uint64_t GenerateRandomNumber();
  void GenerateRandomBlock(uint8_t *buf, int len);
  void GenerateRandomKey(Key *key);


  int Encrypt(const uint8_t *key, int keyLen, const uint8_t *inBuf, int inBufLen,
              uint8_t *outBuf, int outBufLen, int *encryptedLength);

  int Encrypt(const Key *key, const uint8_t *inBuf, int inBufLen,
              uint8_t *outBuf, int outBufLen, int *encryptedLength);

  int Encrypt(const uint8_t *key, int keyLen, struct iovec *inIov, int inIovLen,
              struct iovec *outIov, int outIovLen, int *encryptedLength);

  int Encrypt(const Key *key, struct iovec *inIov, int inIovLen,
              struct iovec *outIov, int outIovLen, int *encryptedLength);

  int Decrypt(const uint8_t *key, int keyLen, const uint8_t *inBuf, int inBufLen,
              uint8_t *outBuf, int outBufLen, int *decryptedLength);

  int Decrypt(const Key *key, const uint8_t *inBuf, int inBufLen,
              uint8_t *outBuf, int outBufLen, int *decryptedLength);

  int Decrypt(const uint8_t *key, int keyLen, struct iovec *inIov, int inIovLen,
              struct iovec *outIov, int outIovLen, int *decryptedLength);

  int Decrypt(const Key *key, struct iovec *inIov, int inIovLen,
              struct iovec *outIov, int outIovLen, int *decryptedLength);

  int GetEncryptedSize(int plainTextSize);
  int GetDecryptedSize(int cipherTextSize);

  // max buffer that is needed for encoded buffer
  // caller of EncodeDataForWritingToKeyFile needs to call
  // this function first to allocate the outbuf.
  // The actual size of encoded data is returned in encodedLength
  int GetEncodedLengthForKeyFileData(int dataLen);
  int EncodeDataForWritingToKeyFile(const uint8_t *serializedProtobuf, int len,
                                 uint8_t *outBuf, int outBufLen,
                                 int *encodedLength);

  // max buffer that is needed for decoded buffer
  // caller of DecodeDataFromKeyFile needs to call
  // this function first to allocate the outbuf.
  // The actual size of encoded data is returned in decodedLength 
  int GetDecodedLengthForKeyFileData(const uint8_t *encodeddata, int encodedDataLen);
  int DecodeDataFromKeyFile(const uint8_t *encodedData, int len,
                            uint8_t *outBuf, int outBufLen,
                            int *decodedLength);
  // Key management functionality
  int SetKeyFile(ServerKeyType keyType, const char* keyFile) {
    int err;
    pthread_mutex_lock(&security_mtx);
    err = keyStore_->SetKeyFile(this, keyType, keyFile);
    pthread_mutex_unlock(&security_mtx);
    return err;
  }  

  int SetKey(ServerKeyType keyType, const Key *key) {
    int err;
    pthread_mutex_lock(&security_mtx);
    err = keyStore_->SetKey(keyType, key);
    pthread_mutex_unlock(&security_mtx);
    return err;
  }

  int GetKey(ServerKeyType keyType, Key *key) {
    int err;
    pthread_mutex_lock(&security_mtx);
    err = keyStore_->GetKey(keyType, key);
    pthread_mutex_unlock(&security_mtx);
    return err;
  } 

  int GenerateTicketAndKey(
    ServerKeyType keyType, const char *userName, uint32_t userNameLen,
    uint32_t uid, uint32_t *gids, uint32_t gidLen, uint64_t expiryTime,
    uint64_t maxRenewalTimeSec, bool isExternal,
    TicketAndKey *ticketAndKey);

  int GenerateTicketAndKeyUsingServerKey(
    ServerKeyType keyType, const Key *key, const char *userName, uint32_t userNameLen,
    uint32_t uid, uint32_t *gids, uint32_t gidLen, uint64_t expiryTime,
    uint64_t maxRenewalTimeSec, bool isExternal,
    TicketAndKey *ticketAndKey);

  int GenerateTicketAndKeyUsingServerKey(
    ServerKeyType keyType, const uint8_t *key, uint32_t keyLen,
    const char *userName, uint32_t userNameLen, uint32_t uid,
    uint32_t *gids, uint32_t gidLen, uint64_t expiryTime,
    uint64_t maxRenewalTimeSec, bool isExternal,
    TicketAndKey *ticketAndKey);

  int GenerateTicketAndKeyUsingServerKey(
    ServerKeyType keyType, const uint8_t *key, uint32_t keyLen,
    const char *userName, uint32_t userNameLen, uint32_t uid,
    uint32_t *gids, uint32_t gidLen, uint64_t expiryTime,
    uint64_t creationTimeSec,
    uint64_t maxRenewalTimeSec, bool isExternal,
    TicketAndKey *ticketAndKey);

  int RenewTicketAndKey(
    ServerKeyType keyType,
    TicketAndKey *inTicketAndKey,
    uint64_t expiryTime,
    TicketAndKey *ticketAndKey);

  bool IsTicketBlacklisted(const Ticket *ticket);
  int GetTicketKeyType(const uint8_t *encryptedTicket, int len,
    ServerKeyType *keyType);
  int DecryptTicket(const uint8_t *encryptedTicket, int len, Ticket *ticket);
  int DecryptTicketAwait(const uint8_t *encryptedTicket, int len, Ticket *ticket);

  // Ticket management functionality
  int SetTicketAndKeyFile(const char* fname) {
    int err;
    pthread_mutex_lock(&security_mtx);
    isTicketSet_ = true;
    err = SetTicketAndKeyFileLockTaken(fname);
    pthread_mutex_unlock(&security_mtx);
    return err;
  }


  int SetTicketAndKey(ServerKeyType keyType,
      const char *clusterName, const TicketAndKey *ticketAndKey) {
    int err;
    pthread_mutex_lock(&security_mtx);
    isTicketSet_ = true;
    err = ticketAndKeyStore_->SetTicketAndKey(keyType,
                                  clusterName, ticketAndKey);  
    pthread_mutex_unlock(&security_mtx);
    return err;
  }

  int GetTicketAndKeyForCluster(
    ServerKeyType keyType,
    const char *clusterName,
    TicketAndKey *ticketAndKey);

   int GetTicketAndKeyEntry(
      ServerKeyType keyType,
      int entryIndex,
      char *outClusterName,
      int outClusterNaleLen,
      TicketAndKey *ticketAndKey);

  // right now we support only SHA256
  int GetHashLen(int hashType) {
    debug_assert(hashType == 0);
    return 32;
  }

  int GetHash(int hashType, uint8_t *inBuf, int inBufLen,
                     uint8_t *outBuf, int outBufLen) {

    if (outBufLen < GetHashLen(hashType)) {
      return ENOSPC;
    }
    CryptoPP::SHA256().CalculateDigest(outBuf, inBuf, inBufLen);
    return 0;
  }
  int GetUserTicketAndKeyFileLocation(uint8_t *pathBuf, int pathBufLen) {
    const char *filePath = getenv("MAPR_TICKETFILE_LOCATION");
    if (filePath) {
      if (strlen(filePath) > (uint32_t) (pathBufLen - 1)) {
        return ENOSPC;
      }
      strncpy((char *)pathBuf, filePath, pathBufLen);
      return 0;
    }

    int len = snprintf((char *) pathBuf, pathBufLen,
                        "%s/maprticket_%s",  mapRDefaultKeyFileLocation_,
                        mapRFileNameSuffix_);

    pathBuf[len] = 0;
    return 0;
  }

  // returns true if the ticketandkey is yet to expire elsee false
  bool IsTicketAndKeyUsable(TicketAndKey *ticketAndKey);
  bool IsTicketExpired(Ticket *ticket);
  
  bool IsKerberosEnabled(const char * clusterName) {
    return maprClusterOptions_.IsUseKerberoseEnabled(clusterName);
  }

  bool IsReplayDetectionDisabled(const char * clusterName) {
    return maprClusterOptions_.IsReplayDetectionDisabled(clusterName);
  }

  int GetCldbHttpsPort(const char * clusterName) {
    return maprClusterOptions_.GetCldbHttpsPort(clusterName);
  }
  
  int GetCldbPrincipal(const char * clusterName, char *value, int n) {
    return maprClusterOptions_.GetCldbPrincipal(clusterName, value, n);
  }
  
  bool IsParsingDone() { 
    return maprClusterOptions_.IsParsingDone();
  }

  void SetParsingDone() {
    maprClusterOptions_.SetParsingDone();
  }

  int SetClusterOption(const char *clusterName,
                       const char *key, const char *value) {
    return maprClusterOptions_.SetClusterOption(clusterName, key, value);
  }

  int GetClusterOption(const char *clusterName,
                       const char *key, char *value, int n) {
    return maprClusterOptions_.GetClusterOption(clusterName, key, value, n);
  }

  void UpdateBlacklistedAe(uint32_t *ids, uint64_t *blacklistTimes,
                           int numUids, bool reinitBlacklist);

  void RemoveBlacklistedAe(uint32_t *uids, int numUids);

  uint64_t GetBlacklistTime(uint32_t id) {
    // should be called only from RPC thread
    debug_assert(GlobalDispatch::GetMyQid() == GlobalDispatch::CpuQ_Rpc);
    return aeHashTable_->GetBlacklistTime(id);
  }

  int AddNewMaprServerTicketEntries(char *maprServerTicketLoc);

private:
  // Make constructor as private to ensure that there is only
  // one instance of this object in the process
  Security();

  ~Security() {
    delete keyStore_;
    delete ticketAndKeyStore_;
    delete aeHashTable_;
  }  
  void InitCredentials();

  // Use hard coded key -- all 'A's to read/write from key file
  void GetKeyForKeyFile(Key *key) {
    uint8_t keybuf[KeySizeInBytes];
    memset(keybuf, 'A', KeySizeInBytes); 
    key->set_key(keybuf, KeySizeInBytes);
  }

  int SetTicketAndKeyFileLockTaken(const char* fname) {
    int err;
    err = ticketAndKeyStore_->SetTicketAndKeyFile(this, fname);
    return err;
  }

  void GetAndUpdateIV(uint8_t *iv, int dataLen);
  uint8_t *GetThreadSpecificIV();
  CryptoPP::AutoSeededRandomPool *GetThreadSpecificRandomPool();
  ThreadSpecificSecurityInfo *GetThreadSpecificSecurityInfo();
  static void FreeThreadSpecificSecurityInfo(void *p);
  static void InitThreadSpecificSecurityInfo();
  static CallbackFunc DecryptAndValidateTicket;

  uid_t uid_;
  uid_t euid_;
  gid_t gid_;
  gid_t egid_;
  gid_t *gids_;
  int numgroups_;

  const char *mapRDefaultKeyFileLocation_;
  // max user name len
  char mapRFileNameSuffix_[256];

  int adminGid_;
  CredentialsMsg creds_;
  KeyStore *keyStore_;
  TicketAndKeyStore *ticketAndKeyStore_;
  bool isTicketSet_;
  AeHashTable *aeHashTable_;

  MaprClusterOptions maprClusterOptions_;
};


}
}

#endif

#include <stdint.h>
#include <string.h>
#include <stdio.h>

namespace mapr {
namespace fs {

struct BlacklistedAeEntry {
  uint32_t id;
  uint64_t blacklistTime;
  struct BlacklistedAeEntry * next;
};

class AeHashTable {
public :
  AeHashTable() {
    table_ = new BlacklistedAeEntry*[NumOfBuckets];
    memset(table_, 0, NumOfBuckets * sizeof (BlacklistedAeEntry*));
  }

  void AddOrModify(uint32_t id, uint64_t blacklistTime);
  uint64_t GetBlacklistTime(uint32_t id);
  void Remove(uint32_t id);

  static const uint32_t NumOfBuckets = 1023;

  ~AeHashTable() {
    BlacklistedAeEntry *entry, *next = NULL;
    for (uint32_t index = 0; index < NumOfBuckets; index++) {
      if ((entry = table_[index])) {
        for (next=entry->next;entry->next;entry=next,next=next->next)
          delete entry;
        delete entry;
      }
    }
  }

private:

  BlacklistedAeEntry **table_;
  
  BlacklistedAeEntry *AeLookup(uint32_t id, uint32_t bucket);

  // TODO check the hash function
  uint32_t HashId(uint32_t id) {
    return id % NumOfBuckets;
  }

};

}
}

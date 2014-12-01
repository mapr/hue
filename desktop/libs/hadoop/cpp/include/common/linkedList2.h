/* */
#ifndef MAPR_LINKEDLIST2
#define MAPR_LINKEDLIST2 // Prevent recursion
#include <stdint.h>
#include <stddef.h>

class linkedListEntry2
{
public:
  linkedListEntry2()
  { /* initialize entry to point to itself */
    this->next = this;
    this->prev = this;
  }
  
  /* insertAfter
     Insert a new linked list entry after an existing entry
     Parameters:
       existing  - entry already in the list
       newEntry  - entry to be inserted into the list after existing
  */
  void  insertAfter(linkedListEntry2& existing, linkedListEntry2& newEntry)
  {
    newEntry.prev       = &existing;
    newEntry.next       = existing.next;
    existing.next->prev = &newEntry;
    existing.next       = &newEntry;
  }
  
  void  insertBefore(linkedListEntry2& existing, linkedListEntry2& newEntry)
  {
    newEntry.next        = &existing;
    newEntry.prev        = existing.prev;
    existing.prev->next  = &newEntry;
    existing.prev        = &newEntry;
  }
  
  void Remove()
  {
    this->next->prev = this->prev;
    this->prev->next = this->next;

    /* returnt to initialized state */
    this->next       = this;
    this->prev       = this;
    
    return;
  }

  linkedListEntry2  *next;
  linkedListEntry2  *prev;
};

class linkedList2
{
public:
  linkedList2()
  { // All initialization done as part of declaration of queueHead
  }

  linkedList2(linkedList2& list)
  {
    linkedListEntry2(list.queueHead);
  }

  void  insertFirst(linkedListEntry2& newEntry)
  {
    this->queueHead.insertAfter(this->queueHead, newEntry);
  }
  
  void  insertLast(linkedListEntry2& newEntry)
  {
    this->queueHead.insertBefore(this->queueHead, newEntry);
  }
  
  linkedListEntry2 *GetFirst()
  {
    return GetNext(this->queueHead);
  }
  
  linkedListEntry2 *GetNext(linkedListEntry2& entry)
  {
    linkedListEntry2 *next = NULL;

    if (entry.next != &this->queueHead)
      next = entry.next;
    
    return next;
  }
  
  linkedListEntry2 *GetLast()
  {
    return GetPrev(this->queueHead);
  }
  
  linkedListEntry2 *GetPrev(linkedListEntry2& entry)
  {
    linkedListEntry2 *prev = NULL;

    if (entry.prev != &this->queueHead)
      prev = entry.prev;
    
    return prev;
  }
  
  void Remove(linkedListEntry2 *entry)
  {
    entry->Remove();
    
    return;
  }

  bool listIsEmpty()
  {
    return (NULL == this->GetFirst());
  }
  
private:
  linkedListEntry2   queueHead;
};

#endif // ifndef MAPR_LINKEDLIST2


/*

  Copyright (C) 1999, 2003 STMicroelectronics, Inc.  All Rights Reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  Written by Thierry Bidault and Christophe Guillon, Jul 2002 
  Contact thierry.bidault@st.com Christophe.guillon@st.com  
*/

/*
 * implementation of simple hash module
 */

#include "myalloc.h"
// [HK] added string.h for mem* prototypes
#include <string.h>
#include <assert.h>
#include "hash.h"

#if !defined(HASH_SELFTEST) && defined(LCC_ALLOC)
#include "c.h"
#define HASH_ALLOC(size) allocate(size, PERM)
#define HASH_FREE(ptr) ((void)0)
#else
#include <stdlib.h>
#define HASH_ALLOC(size) MYMALLOC(size)
#define HASH_FREE(ptr) MYFREE(ptr)
#endif

#ifdef HASH_DEBUG
#define HASH_ASSERT(cond) assert(cond)
#else
#define HASH_ASSERT(cond) ((void)0)
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

struct hash_bucket_s {
  const char *key;
  int key_size;
  struct hash_bucket_s *next;
  long long user[1];
};
#define hash_bucket_SIZE(elt_size) (sizeof(struct hash_bucket_s)-sizeof(long long)+(elt_size))

static void
hash_bucket_dtor(hash_bucket_t bucket) {
}

void *
hash_bucket_data(hash_bucket_t bucket)
{
  return (void *)((char *)bucket + sizeof(struct hash_bucket_s) - sizeof(long long));
}

const char *
hash_bucket_key(hash_bucket_t bucket)
{
  return bucket->key;
}

int
hash_bucket_key_size(hash_bucket_t bucket)
{
  return bucket->key_size;
}

#define REHASH_LIMIT 0.65

struct hash_s
{
  unsigned size;
  unsigned elt_size;
  unsigned count;
  int rehash;
  char *buckets;
};

#define hash_SIZE(size, elt_size) (sizeof(struct hash_s))
#define hash_SIZEOF(this) (sizeof(struct hash_s))

static hash_bucket_t
hash_bucket(hash_t hash, unsigned index)
{
  return (hash_bucket_t)(hash->buckets + index * hash_bucket_SIZE(hash->elt_size));
}

#if 0
static int
bitcount (int mask)
{
  int i;
  int bits = 0;
  for (i = 0; i < sizeof(mask)*8; i++)
    if (1<<i & mask)
      bits++;
  return bits;
}
#endif
static void
hash_swap(hash_t this, hash_t that)
{
  struct hash_s tmp_;
  HASH_ASSERT(hash_SIZEOF(that) == hash_SIZEOF(this));
  memcpy(&tmp_, that, hash_SIZEOF(that));
  memcpy(that, this,  hash_SIZEOF(that));
  memcpy(this, &tmp_, hash_SIZEOF(that));
}

static void
hash_ctor(hash_t hash, int size, int elt_size)
{
  char *buckets;
  int alloc_size;

  if (size == 0) 
    size = 128;
  HASH_ASSERT(bitcount(size) == 1);
  
  /* Allocates buckets and set to zero.  */
  alloc_size = size * hash_bucket_SIZE(elt_size);
  buckets = (char *)HASH_ALLOC(alloc_size);
  memset (buckets, 0, alloc_size);

  hash->size = size;
  hash->elt_size = elt_size;
  hash->count = 0;
  hash->rehash = 1;
  hash->buckets = buckets;
}

static void
hash_dtor(hash_t hash)
{
  int i;
  for (i = 0; i < hash->size; i++) {
    hash_bucket_t bucket = hash_bucket(hash, i);
    if (bucket->key && bucket->next) {
      bucket = bucket->next;
      while(bucket && bucket->key) {
	hash_bucket_t next = bucket->next;
	hash_bucket_dtor(bucket);
	HASH_FREE(bucket);
	bucket = next;
      }
    }
  }
  HASH_FREE(hash->buckets);
}

hash_t
hash_new (int size, int elt_size)
{
  hash_t hash;

  /* Allocates and constructs.  */
  hash = (hash_t)HASH_ALLOC(hash_SIZE(size, elt_size));
  hash_ctor(hash, size, elt_size);
  return hash;
}

hash_t
hash_del (hash_t hash)
{
  hash_dtor(hash);
  HASH_FREE(hash);
  return NULL;
}

static void hash_rehash(hash_t hash)
{
  hash_iterator_t iterator;
  struct hash_s new_hash_;

  hash_ctor(&new_hash_, hash->size<<1, hash->elt_size);

  for(iterator = hash_iterator_new(hash); 
      !hash_iterator_at_end(iterator); 
      hash_iterator_advance(iterator)) {
    hash_bucket_t bucket = hash_iterator_current(iterator);
    hash_bucket_t new_bucket = hash_insert_bucket(&new_hash_, hash_bucket_key(bucket), hash_bucket_key_size(bucket));
    memcpy(hash_bucket_data(new_bucket), hash_bucket_data(bucket), hash->elt_size);
  }
  hash_iterator_del(iterator);
  hash_swap(hash, &new_hash_);
  hash_dtor(&new_hash_);
}

static float hash_ratio(hash_t hash)
{
  return (float)hash->count / hash->size; 
}

int hash_count(hash_t hash)
{
  return hash->count;
}

int hash_size(hash_t hash)
{
  return hash->size;
}

int hash_used_slots(hash_t hash)
{
  int c = 0;
  int i;
  for (i = 0; i < hash->size; i++) {
    if (hash_bucket(hash, i)->key != NULL) c++;
  }
  return c;
}
/*#define HASH32*/
#ifdef HASH32
static unsigned
hash_key(const char *str, int size)
{
  const unsigned char *high = (const unsigned char *)(str + size);
  const unsigned char *low = (const unsigned char *)str;
  register unsigned hashval = 0, previous = 0, i;
  register const unsigned char *ptr = low;
  for (i = 0; ptr + 4 <= high; ptr += 4, i++) {
    unsigned current = ((ptr[0]) + (ptr[1]<<8)) + ((ptr[2]<<16) + (ptr[3]<<24));
    hashval += (current>>i) + (previous<<(32-i));
    previous = current;
  }
  if (ptr + 3 <= high) {
    unsigned current = ((ptr[0]) + (ptr[1]<<8)) + ((ptr[2]<<16));
    hashval += (current>>i) + (previous<<(32-i));
  } else if (ptr + 2 <= high) {
    unsigned current = ((ptr[0]) + (ptr[1]<<8));
    hashval += (current>>i) + (previous<<(32-i));
  } else if (ptr + 1 <= high) {
    unsigned current = ((ptr[0]));
    hashval += (current>>i) + (previous<<(32-i));
  }
  return hashval;
}
#else
/*
 * Hashing rand table.
 */
static const unsigned int rand_table[256] = {
#include "rand.tbl.c"
};
static unsigned

hash_key(const char *str, int size)
{
  int i;
  unsigned h = 0;
  for (i = 0; i < size; i++) {
    h += rand_table[(unsigned char)str[i]];
  } 
  return h;
}
#endif

hash_bucket_t
hash_insert_bucket (hash_t hash, const char *key, int key_size)
{
  unsigned hashed;
  hash_bucket_t bucket, pbucket;
  HASH_ASSERT(hash != NULL);
  hashed= hash_key (key, key_size);
  /* Size is a power of two.  */
  bucket = hash_bucket(hash, hashed & (hash->size-1));
  for (pbucket = bucket; 
       (bucket &&
	bucket->key);
       pbucket = bucket, bucket = bucket->next)
    if (bucket->key_size == key_size &&
	memcmp (bucket->key, key, key_size) == 0)
      break;

  if (bucket == NULL) {
    int alloc_size = hash_bucket_SIZE(hash->elt_size);
    bucket = (hash_bucket_t)HASH_ALLOC (alloc_size);
    memset (bucket, 0, alloc_size);
    pbucket->next = bucket;
  }
  if (bucket->key == NULL) {
    hash->count++;
    bucket->key = key;
    bucket->key_size = key_size;
    if (hash->rehash && hash_ratio(hash) > REHASH_LIMIT) {
      hash_rehash(hash);
      bucket = hash_access_bucket(hash, key, key_size);
    }
  }
  return bucket;
}

hash_bucket_t
hash_access_bucket (hash_t hash, const char *key, int key_size)
{
  unsigned hashed;
  hash_bucket_t bucket;
  HASH_ASSERT(hash != NULL);
  hashed= hash_key (key, key_size);
  /* Size is a power of two.  */
  bucket = hash_bucket(hash, hashed & (hash->size-1));
  for (;(bucket &&
	 bucket->key);
       bucket = bucket->next)
    if (bucket->key_size == key_size &&
	memcmp (bucket->key, key, key_size) == 0)
      break;
  if (bucket == NULL || bucket->key == NULL)
    return NULL;
  return bucket;
}

hash_bucket_t
hash_remove_bucket (hash_t hash, const char *key, int key_size)
{
  unsigned hashed;
  hash_bucket_t bucket, pbucket;
  HASH_ASSERT(hash != NULL);
  hashed= hash_key (key, key_size);
  /* Size is a power of two.  */
  bucket = hash_bucket(hash, hashed & (hash->size-1));
  for (pbucket = bucket; 
       (bucket &&
	bucket->key);
       pbucket = bucket, bucket = bucket->next)
    if (bucket->key_size == key_size &&
	memcmp (bucket->key, key, key_size) == 0)
      break;

  if (bucket == NULL || bucket->key == NULL) {
    return NULL;
  }
  pbucket->next = bucket->next;
  bucket->key = NULL;
  bucket->key_size = 0;
  bucket->next = NULL;
  hash->count--;
  return bucket;
}

struct hash_iterator_s {
  hash_t hash;
  unsigned index;
  hash_bucket_t bucket;
};

hash_iterator_t
hash_iterator_new(hash_t hash)
{
  hash_iterator_t iterator;
  iterator = (hash_iterator_t)MYMALLOC(sizeof(*iterator));
  iterator->hash = hash;
  hash_iterator_rewind(iterator);
  return iterator;
}

hash_iterator_t
hash_iterator_del(hash_iterator_t iterator)
{
  if (iterator != NULL) MYFREE(iterator);
  return NULL;
}

void
hash_iterator_advance(hash_iterator_t iterator)
{
  if (iterator->bucket != NULL) {
    iterator->bucket = iterator->bucket->next;
    if (iterator->bucket == NULL) iterator->index++;
  }
  if (iterator->bucket == NULL) {
    while (iterator->index < iterator->hash->size) {
      hash_bucket_t local_bucket = hash_bucket(iterator->hash, iterator->index);
      if (local_bucket->key != NULL) {
	iterator->bucket = local_bucket;
	break;
      }
      iterator->index++;
    }
  }
}

int
hash_iterator_at_end(hash_iterator_t iterator)
{
  return iterator->bucket == NULL;
}

hash_bucket_t
hash_iterator_current(hash_iterator_t iterator)
{
  return iterator->bucket;
}

void
hash_iterator_rewind(hash_iterator_t iterator)
{
  iterator->index = 0;
  iterator->bucket = NULL;
  hash_iterator_advance(iterator);
}

#ifdef HASH_SELFTEST
#include <stdio.h>
#define TEST_KEY(key) if ((bucket = hash_access_bucket(hash, (const char*)key, sizeof(long)))) { \
    elt_test = hash_bucket_data(bucket); \
    printf ("key %ld found (%d, %d)\n", *key, elt_test->first, elt_test->elt_size ); \
  } else { \
    printf ("key: %ld not found\n", *key); \
  }

typedef struct elt_type {
  int count;
  int first;
  int last;
  int size;
  int elt_size;
} elt_type;

elt_type elt_type_dup(elt_type elt)
{
  elt_type *pelt = (elt_type*)MYMALLOC(sizeof(elt_type));
  memcpy(pelt, &elt, sizeof(elt_type));
  return *pelt;
}
hash_t hash;
int 
main1(int argc, char *argv[]) {
   /* test iteration. */
  {
    hash_iterator_t iterator;
    printf("iteration:\n");
    for(iterator = hash_iterator_new(hash); 
	!hash_iterator_at_end(iterator); 
	hash_iterator_advance(iterator)) {
      hash_bucket_t bucket = hash_iterator_current(iterator);
      long *key = (long *)hash_bucket_key(bucket);
      elt_type *elt = hash_bucket_data(bucket);
      printf ("key %ld  (%d, %d)\n", *key, elt->first, elt->elt_size ); \
    }
    hash_iterator_del(iterator);
  }
  /* statistics. */
  printf("count %d\n", hash_count(hash));
  printf("size %d\n", hash_size(hash));
  printf("used %d\n", hash_used_slots(hash));
  printf("ratio %f\n", hash_ratio(hash));
  return 0;
}

int 
main2(int argc, char *argv[]) {
  hash_bucket_t bucket;
  long *key1 , *key2 , *key3 , *key4 , *key5;
  elt_type base;
  elt_type *elt, *elt_test;

  base.first = 88;
  base.elt_size = 99;
  key1 = (long*)MYMALLOC(sizeof(long));
  key2 = (long*)MYMALLOC(sizeof(long));
  key3 = (long*)MYMALLOC(sizeof(long));
  key4 = (long*)MYMALLOC(sizeof(long));
  key5 = (long*)MYMALLOC(sizeof(long));
  *key1 = 1;
  *key2 = 2;
  *key3 = 3;
  *key4 = 4;
  *key5 = 5;
  bucket = hash_insert_bucket(hash, (const char*)key1, sizeof(long));
  elt = hash_bucket_data(bucket);
  *elt = /* elt_type_dup */(base);
  bucket = hash_insert_bucket(hash, (const char*)key2, sizeof(long));
  elt = hash_bucket_data(bucket);
  elt->first=77;
  bucket = hash_insert_bucket(hash, (const char*)key3, sizeof(long));
  bucket = hash_insert_bucket(hash, (const char*)key4, sizeof(long));
  TEST_KEY(key1);
  TEST_KEY(key2);
  TEST_KEY(key3);
  TEST_KEY(key4);
  TEST_KEY(key5);
  return 0;
/*   return main1(argc, argv); */
}

int 
main(int argc, char *argv[]) {
  hash = hash_new(4, sizeof(elt_type));
  main2(argc, argv);
  main1(argc,argv);
  hash_del(hash);
  return 0;
}
#endif


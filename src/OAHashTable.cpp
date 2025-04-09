/**
 * @file OAHashTable.cpp
 * @author Edgar Jose Donoso Mansilla (e.donosomansilla)
 * @course CS280
 * @term Spring 2025
 *
 * @brief Implementation of a basic hash table
 */

#pragma once

#include <cstddef>
#include <cstring>
#include <iostream>
#include <ostream>
#define OAHASHTABLE_CPP

#ifndef OAHASHTABLEH
  #include "OAHashTable.h"
#endif

template<typename T>
OAHashTable<T>::OAHashTable(const OAHTConfig& Config):
    config(Config),
    slots(new OAHTSlot[Config.InitialTableSize_]),
    first_hash_function(config.PrimaryHashFunc_),
    second_hash_function(config.SecondaryHashFunc_),
    delete_function(config.FreeProc_),
    stats() {
  stats.TableSize_ = config.InitialTableSize_;
  stats.PrimaryHashFunc_ = first_hash_function;
  stats.SecondaryHashFunc_ = second_hash_function;

  InitTable();
}

template<typename T>
OAHashTable<T>::~OAHashTable() {
  // TODO: Impelement destructor
}

template<typename T>
auto OAHashTable<T>::insert(const char* Key, const T& Data) -> void {

  // NOTE: Expand table if needed

  // TODO: Impelement insertion
  std::size_t index = first_hash_function(Key, stats.TableSize_);
  OAHTSlot* slot = &slots[index];

  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    std::size_t wrapped_index = (index + i) % stats.TableSize_;
    stats.Probes_++;

    if (slots[wrapped_index].State == OAHashTable::OAHTSlot::UNOCCUPIED) {
      slot = &slots[wrapped_index];
      slot->probes = i;
      break;
    }

  }

  slot->State = OAHashTable::OAHTSlot::OCCUPIED;
  strcpy(slot->Key, Key);
  slot->Data = Data;

  stats.Count_++;
}

template<typename T>
auto OAHashTable<T>::remove(const char* Key) -> void {
  // TODO: Impelement deletion
}

template<typename T>
auto OAHashTable<T>::find(const char* Key) const -> const T& {
  // TODO: Impelement search
}

template<typename T>
auto OAHashTable<T>::clear() -> void {
  // TODO: Impelement clearing
}

template<typename T>
auto OAHashTable<T>::GetStats() const -> OAHTStats {
  return stats;
}

template<typename T>
auto OAHashTable<T>::GetTable() const -> const OAHTSlot* {
  return slots;
}

template<typename T>
auto OAHashTable<T>::InitTable() -> void {
  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    slots[i].Key[0] = '\0';
    slots[i].Data = T();
    slots[i].State = OAHashTable::OAHTSlot::UNOCCUPIED;
    slots[i].probes = 0;
  }
}

template<typename T>
auto OAHashTable<T>::GrowTable() -> void {
  // TODO: Table extending
}

template<typename T>
auto OAHashTable<T>::IndexOf(const char* Key, OAHTSlot*& Slot) const -> int {
  // TODO: Table indexing
}

// Stats stuff

OAHTStats::OAHTStats():
    Count_(0),
    TableSize_(0),
    Probes_(0),
    Expansions_(0),
    PrimaryHashFunc_(0),
    SecondaryHashFunc_(0) {};

// Config stuff

template<typename T>
OAHashTable<T>::OAHTConfig::OAHTConfig(
  unsigned InitialTableSize,
  HASHFUNC PrimaryHashFunc,
  HASHFUNC SecondaryHashFunc,
  double MaxLoadFactor,
  double GrowthFactor,
  OAHTDeletionPolicy Policy,
  FREEPROC FreeProc
):
    InitialTableSize_(InitialTableSize),
    PrimaryHashFunc_(PrimaryHashFunc),
    SecondaryHashFunc_(SecondaryHashFunc),
    MaxLoadFactor_(MaxLoadFactor),
    GrowthFactor_(GrowthFactor),
    DeletionPolicy_(Policy),
    FreeProc_(FreeProc) {}

// Exception functions

OAHashTableException::OAHashTableException(
  int ErrCode,
  const std::string& Message
):
    error_code_(ErrCode), message_(Message) {};

OAHashTableException::~OAHashTableException() {}

int OAHashTableException::code() const { return error_code_; }

const char* OAHashTableException::what() const { return message_.c_str(); }

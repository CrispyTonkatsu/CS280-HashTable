/**
 * @file OAHashTable.cpp
 * @author Edgar Jose Donoso Mansilla (e.donosomansilla)
 * @course CS280
 * @term Spring 2025
 *
 * @brief Implementation of a basic hash table
 */

#pragma once

#include <cmath>
#include <cstring>

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

  init_table();
}

template<typename T>
OAHashTable<T>::~OAHashTable() {
  // TODO: Impelement destructor
}

template<typename T>
auto OAHashTable<T>::insert(const char* Key, const T& Data) -> void {
  try_grow_table();

  std::size_t index = first_hash_function(Key, stats.TableSize_);
  OAHTSlot* slot = nullptr;

  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    slot = &get_slot_mut(index + i);

    if (slot->State == OAHashTable::OAHTSlot::UNOCCUPIED) {
      break;
    }
  }

  if (slot == nullptr) {
    throw OAHashTableException(
      OAHashTableException::E_NO_MEMORY,
      "There is not slot available."
    );
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
  std::size_t index = first_hash_function(Key, stats.TableSize_);

  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    const OAHTSlot& slot = get_slot(index + i);

    if (slot.State == OAHashTable::OAHTSlot::UNOCCUPIED) {
      break;
    }

    if (strcmp(slot.Key, Key) == 0) {
      return slot.Data;
    }
  }

  throw OAHashTableException(
    OAHashTableException::E_ITEM_NOT_FOUND,
    "Item not found in table."
  );
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
auto OAHashTable<T>::init_table() -> void {
  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    slots[i].Key[0] = '\0';
    slots[i].Data = T();
    slots[i].State = OAHashTable::OAHTSlot::UNOCCUPIED;
    slots[i].probes = 0;
  }
}

template<typename T>
auto OAHashTable<T>::try_grow_table() -> void {
  const float load_factor =
    static_cast<float>(stats.Count_ + 1) / static_cast<float>(stats.TableSize_);

  if (load_factor < config.MaxLoadFactor_) {
    return;
  }

  double new_factor = std::ceil(stats.TableSize_ * config.GrowthFactor_);
  unsigned new_size = GetClosestPrime(static_cast<unsigned>(new_factor));

  size_t old_size = stats.TableSize_;
  stats.TableSize_ = new_size;
  stats.Count_ = 0;

  OAHTSlot* old_slots = slots;
  slots = new OAHTSlot[new_size];

  for (std::size_t i = 0; i < old_size; i++) {
    if (old_slots[i].State == OAHashTable::OAHTSlot::OCCUPIED) {
      insert(old_slots[i].Key, old_slots[i].Data);
    }
  }

  stats.Expansions_++;
}

template<typename T>
auto OAHashTable<T>::index_of(const char* Key, OAHTSlot*& Slot) const -> int {
  // TODO: Table indexing
}

template<typename T>
auto OAHashTable<T>::get_slot(std::size_t index) const -> const OAHTSlot& {
  std::size_t wrapped_index = index % stats.TableSize_;
  OAHTSlot& slot = slots[wrapped_index];

  slot.probes++;
  stats.Probes_++;

  return slot;
}

template<typename T>
auto OAHashTable<T>::get_slot_mut(std::size_t index) -> OAHTSlot& {
  std::size_t wrapped_index = index % stats.TableSize_;
  OAHTSlot& slot = slots[wrapped_index];

  slot.probes++;
  stats.Probes_++;

  return slot;
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

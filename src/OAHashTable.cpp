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
#include <cstddef>
#include <cstring>

#include "Support.h"

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
OAHashTable<T>::OAHashTable(const OAHashTable& rhs):
    config(rhs.config),
    slots(nullptr),
    first_hash_function(rhs.first_hash_function),
    second_hash_function(rhs.second_hash_function),
    delete_function(rhs.delete_function),
    stats() {}

template<typename T>
OAHashTable<T>::OAHashTable(OAHashTable&& rhs) {}

template<typename T>
auto OAHashTable<T>::operator=(const OAHashTable& rhs) -> OAHashTable& {}

template<typename T>
auto OAHashTable<T>::operator=(OAHashTable&& rhs) -> OAHashTable& {}

template<typename T>
OAHashTable<T>::~OAHashTable() {
  clear();
  delete[] slots;
}

template<typename T>
auto OAHashTable<T>::insert(const char* Key, const T& Data) -> void {
  insert_inner(Key, Data);
}

template<typename T>
auto OAHashTable<T>::remove(const char* Key) -> void {
  SlotSearch<OAHTSlot> search = find_slot_mut(Key);

  if (search.slot == nullptr) {
    throw OAHashTableException(
      OAHashTableException::E_ITEM_NOT_FOUND,
      "Key not in table."
    );
  }

  delete_slot(*search.slot);

  switch (config.DeletionPolicy_) {
    case OAHTDeletionPolicy::MARK: adjust_mark(search.index); break;
    case OAHTDeletionPolicy::PACK: adjust_pack(search.index); break;
  }
}

template<typename T>
auto OAHashTable<T>::find(const char* Key) const -> const T& {
  SlotSearch<const OAHTSlot> search{find_slot(Key)};

  if (search.slot == nullptr) {
    throw OAHashTableException(
      OAHashTableException::E_ITEM_NOT_FOUND,
      "Item not found in table."
    );
  }

  return search.slot->Data;
}

template<typename T>
auto OAHashTable<T>::clear() -> void {
  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    OAHTSlot& slot = get_slot_mut(i, false).slot;

    switch (slot.State) {
      case OAHashTable::OAHTSlot::UNOCCUPIED: break;
      case OAHashTable::OAHTSlot::OCCUPIED: delete_slot(slot); break;
      case OAHashTable::OAHTSlot::DELETED:
        slot.State = OAHashTable::OAHTSlot::UNOCCUPIED;
        break;
    }
  }
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
auto OAHashTable<T>::init_table(bool reset_probes) -> void {
  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    slots[i].Key[0] = '\0';
    slots[i].Data = T();
    slots[i].State = OAHashTable::OAHTSlot::UNOCCUPIED;

    if (reset_probes) {
      slots[i].probes = 0;
    }
  }
}

template<typename T>
auto OAHashTable<T>::try_grow_table() -> void {
  const float load_factor =
    static_cast<float>(stats.Count_ + 1) / static_cast<float>(stats.TableSize_);

  if (load_factor <= config.MaxLoadFactor_) {
    return;
  }

  double new_factor = std::ceil(stats.TableSize_ * config.GrowthFactor_);
  unsigned new_size = GetClosestPrime(static_cast<unsigned>(new_factor));

  size_t old_size = stats.TableSize_;
  stats.TableSize_ = new_size;
  stats.Count_ = 0;

  OAHTSlot* old_slots = slots;
  slots = new OAHTSlot[new_size];
  init_table(true);

  for (std::size_t i = 0; i < old_size; i++) {
    if (old_slots[i].State == OAHashTable::OAHTSlot::OCCUPIED) {
      insert(old_slots[i].Key, old_slots[i].Data);
    }
  }

  delete[] old_slots;

  stats.Expansions_++;
}

template<typename T>
auto OAHashTable<T>::insert_inner(const char* Key, const T& Data, bool probe)
  -> void {
  try_grow_table();

  std::size_t index = first_hash_function(Key, stats.TableSize_);
  OAHTSlot* slot = nullptr;

  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    slot = &get_next_slot_mut(Key, index, i, probe);

    if (slot->State == OAHashTable::OAHTSlot::OCCUPIED) {
      if (strcmp(slot->Key, Key) == 0) {
        throw OAHashTableException(
          OAHashTableException::E_DUPLICATE,
          "There is a duplicate item in the list."
        );
      }
      continue;
    }

    if (slot->State == OAHashTable::OAHTSlot::UNOCCUPIED) {
      break;
    }

    for (std::size_t j = i + 1; j < stats.TableSize_; j++) {
      const OAHTSlot& next_slot = get_next_slot(Key, index, j, probe);

      if (next_slot.State == OAHashTable::OAHTSlot::UNOCCUPIED) {
        break;
      }

      if (strcmp(next_slot.Key, Key) == 0) {
        throw OAHashTableException(
          OAHashTableException::E_DUPLICATE,
          "There is a duplicate item in the list."
        );
      }
    }

    break;
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
auto OAHashTable<T>::find_slot(const char* Key) const
  -> const SlotSearch<const OAHTSlot> {
  std::size_t index = first_hash_function(Key, stats.TableSize_);

  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    const OAHTSlot& slot = get_next_slot(Key, index, i);

    if (slot.State == OAHashTable::OAHTSlot::UNOCCUPIED) {
      break;
    }

    if (strcmp(slot.Key, Key) == 0) {
      std::size_t wrapped_index = (index + i) % stats.TableSize_;
      return SlotSearch<const OAHTSlot>{wrapped_index, &slot};
    }
  }

  return SlotSearch<const OAHTSlot>{};
}

template<typename T>
auto OAHashTable<T>::find_slot_mut(const char* Key)
  -> const SlotSearch<OAHTSlot> {
  std::size_t index = first_hash_function(Key, stats.TableSize_);

  for (std::size_t i = 0; i < stats.TableSize_; i++) {
    SlotProbe<OAHTSlot> query = get_next_slot_mut_with_index(Key, index, i);
    OAHTSlot& slot = query.slot;

    if (slot.State == OAHashTable::OAHTSlot::UNOCCUPIED) {
      break;
    }

    if (strcmp(slot.Key, Key) == 0) {
      return SlotSearch<OAHTSlot>{query.index, &slot};
    }
  }

  return SlotSearch<OAHTSlot>{};
}

template<typename T>
template<typename S>
OAHashTable<T>::SlotProbe<S>::SlotProbe(std::size_t index, S& slot):
    index(index), slot(slot) {}

template<typename T>
auto OAHashTable<T>::get_slot(std::size_t index, bool probe) const
  -> const SlotProbe<const OAHTSlot> {
  std::size_t wrapped_index = index % stats.TableSize_;
  OAHTSlot& slot = slots[wrapped_index];

  if (probe) {
    slot.probes++;
    stats.Probes_++;
  }

  return SlotProbe<const OAHTSlot>(wrapped_index, slot);
}

template<typename T>
auto OAHashTable<T>::get_slot_mut(std::size_t index, bool probe)
  -> const SlotProbe<OAHTSlot> {
  std::size_t wrapped_index = index % stats.TableSize_;
  OAHTSlot& slot = slots[wrapped_index];

  if (probe) {
    slot.probes++;
    stats.Probes_++;
  }

  return SlotProbe<OAHTSlot>(wrapped_index, slot);
}

template<typename T>
auto OAHashTable<T>::use_secondary_hash(const char* Key) const -> std::size_t {
  if (second_hash_function == nullptr) {
    return 0;
  }

  return second_hash_function(Key, stats.TableSize_ - 1) + 1;
}

template<typename T>
auto OAHashTable<T>::get_next_slot_with_index(
  const char* Key,
  std::size_t index,
  std::size_t offset,
  bool probe
) const -> const SlotProbe<const OAHTSlot> {
  std::size_t next = index;

  if (second_hash_function != nullptr) {

    if (offset != 0) {
      next += offset * use_secondary_hash(Key);
    }

  } else {
    next += offset;
  }

  return get_slot(next, probe);
}

template<typename T>
auto OAHashTable<T>::get_next_slot_mut_with_index(
  const char* Key,
  std::size_t index,
  std::size_t offset,
  bool probe
) -> const SlotProbe<OAHTSlot> {
  std::size_t next = index;

  if (second_hash_function != nullptr) {

    if (offset != 0) {
      next += offset * use_secondary_hash(Key);
    }

  } else {
    next += offset;
  }

  return get_slot_mut(next, probe);
}

template<typename T>
auto OAHashTable<T>::get_next_slot(
  const char* Key,
  std::size_t index,
  std::size_t offset,
  bool probe
) const -> const OAHTSlot& {
  return get_next_slot_with_index(Key, index, offset, probe).slot;
}

template<typename T>
auto OAHashTable<T>::get_next_slot_mut(
  const char* Key,
  std::size_t index,
  std::size_t offset,
  bool probe
) -> OAHTSlot& {
  return get_next_slot_mut_with_index(Key, index, offset, probe).slot;
}

template<typename T>
auto OAHashTable<T>::adjust_mark(std::size_t index) -> void {
  slots[index].State = OAHashTable::OAHTSlot::DELETED;
}

template<typename T>
auto OAHashTable<T>::adjust_pack(std::size_t index) -> void {
  for (std::size_t i = 1; i < stats.TableSize_; i++) {
    OAHTSlot& slot = get_slot_mut(index + i, false).slot;

    if (slot.State != OAHashTable::OAHTSlot::OCCUPIED) {
      break;
    }

    slot.State = OAHashTable::OAHTSlot::UNOCCUPIED;
    stats.Count_--;
    insert_inner(slot.Key, slot.Data);
  }
}

template<typename T>
auto OAHashTable<T>::delete_slot(OAHTSlot& slot) -> void {
  if (slot.State != OAHashTable::OAHTSlot::OCCUPIED) {
    return;
  }

  if (delete_function != nullptr) {
    delete_function(slot.Data);
  }

  slot.State = OAHashTable::OAHTSlot::UNOCCUPIED;
  stats.Count_--;
}

// Stats stuff

OAHTStats::OAHTStats():
    Count_(0),
    TableSize_(0),
    Probes_(0),
    Expansions_(0),
    PrimaryHashFunc_(0),
    SecondaryHashFunc_(0) {}

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
    error_code_(ErrCode), message_(Message) {}

OAHashTableException::~OAHashTableException() {}

int OAHashTableException::code() const { return error_code_; }

const char* OAHashTableException::what() const { return message_.c_str(); }

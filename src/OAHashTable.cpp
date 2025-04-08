/**
 * @file OAHashTable.cpp
 * @author Edgar Jose Donoso Mansilla (e.donosomansilla)
 * @course CS280
 * @term Spring 2025
 *
 * @brief Implementation of a basic hash table
 */

#define OAHASHTABLE_CPP

#ifndef OAHASHTABLEH
  #include "OAHashTable.h"
#endif

template<typename T>
OAHashTable<T>::OAHashTable(const OAHTConfig& Config):
    config(Config),
    slots(new OAHTSlot[config.InitialTableSize_]),
    first_hash_function(config.PrimaryHashFunc_),
    second_hash_function(config.SecondaryHashFunc_),
    delete_function(config.FreeProc_),
    stats() {}

template<typename T>
OAHashTable<T>::~OAHashTable() {
  // TODO: Impelement destructor
}

template<typename T>
auto OAHashTable<T>::insert(const char* Key, const T& Data) -> void {
  // TODO: Impelement insertion
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
  // TODO: Table getter
  return nullptr;
}

template<typename T>
auto OAHashTable<T>::InitTable() -> void {
  // TODO: Table initialization
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

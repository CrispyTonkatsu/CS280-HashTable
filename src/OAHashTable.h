/**
 * @file OAHashTable.h
 * @author Edgar Jose Donoso Mansilla (e.donosomansilla)
 * @course CS280
 * @term Spring 2025
 *
 * @brief Implementation of a basic hash table
 */

#pragma once

//---------------------------------------------------------------------------
#include <string>

#ifndef OAHASHTABLEH
  #define OAHASHTABLEH
//---------------------------------------------------------------------------

/*!
client-provided hash function: takes a key and table size,
returns an index in the table.
*/
typedef unsigned (*HASHFUNC)(const char*, unsigned);

//! Max length of our "string" keys
const unsigned MAX_KEYLEN = 32;

//! The exception class for the hash table
class OAHashTableException {
private:

  int error_code_;      //!< Code for the exception
  std::string message_; //!< Readable string describing the exception

public:

  //! Non-default constructor
  OAHashTableException(int ErrCode, const std::string& Message);

  //! Destructor
  virtual ~OAHashTableException();

  /*!
    Retrieves exception code

    \return
      One of: E_ITEM_NOT_FOUND, E_DUPLICATE, E_NO_MEMORY
  */
  virtual int code() const;

  /*!
    Retrieve human-readable string describing the exception

    \return
      The description of the exception
  */
  virtual const char* what() const;

  //! Possible exception conditions
  enum OAHASHTABLE_EXCEPTION {
    E_ITEM_NOT_FOUND,
    E_DUPLICATE,
    E_NO_MEMORY
  };
};

//! The policy used during a deletion
enum OAHTDeletionPolicy {
  MARK,
  PACK
};

//! OAHashTable statistical info
struct OAHTStats {
  //! Default constructor
  OAHTStats();
  unsigned Count_;             //!< Number of elements in the table
  unsigned TableSize_;         //!< Size of the table (total slots)
  unsigned Probes_;            //!< Number of probes performed
  unsigned Expansions_;        //!< Number of times the table grew
  HASHFUNC PrimaryHashFunc_;   //!< Pointer to primary hash function
  HASHFUNC SecondaryHashFunc_; //!< Pointer to secondary hash function
};

//! Hash table definition (open-addressing)
template<typename T>
class OAHashTable {
public:

  typedef void (*FREEPROC)(T); //!< client-provided free proc (we own the data)

  //! Configuration for the hash table
  struct OAHTConfig {
    //! Non-default constructor
    OAHTConfig(
      unsigned InitialTableSize,
      HASHFUNC PrimaryHashFunc,
      HASHFUNC SecondaryHashFunc = 0,
      double MaxLoadFactor = 0.5,
      double GrowthFactor = 2.0,
      OAHTDeletionPolicy Policy = PACK,
      FREEPROC FreeProc = 0
    );

    unsigned InitialTableSize_;         //!< The starting table size
    HASHFUNC PrimaryHashFunc_;          //!< First hash function
    HASHFUNC SecondaryHashFunc_;        //!< Hash function to resolve collisions
    double MaxLoadFactor_;              //!< Maximum LF before growing
    double GrowthFactor_;               //!< The amount to grow the table
    OAHTDeletionPolicy DeletionPolicy_; //!< MARK or PACK
    FREEPROC FreeProc_;                 //!< Client-provided free function
  };

  //! Slots that will hold the key/data pairs
  struct OAHTSlot {
    //! The 3 possible states the slot can be in
    enum OAHTSlot_State {
      OCCUPIED,
      UNOCCUPIED,
      DELETED
    };

    char Key[MAX_KEYLEN]{'\0'};       //!< Key is a string
    T Data{};                         //!< Client data
    OAHTSlot_State State{UNOCCUPIED}; //!< The state of the slot
    int probes{0};                    //!< For testing
  };

  OAHashTable(const OAHTConfig& Config); // Constructor
  ~OAHashTable();                        // Destructor

  // TODO: Rule of 5

  OAHashTable(const OAHashTable& rhs) = delete;
  OAHashTable(OAHashTable&& rhs) = delete;

  OAHashTable& operator=(const OAHashTable& rhs) = delete;
  OAHashTable& operator=(OAHashTable&& rhs) = delete;

  // Insert a key/data pair into table. Throws an exception if the
  // insertion is unsuccessful.
  void insert(const char* Key, const T& Data);

  // Delete an item by key. Throws an exception if the key doesn't exist.
  // Compacts the table by moving key/data pairs, if necessary
  void remove(const char* Key);

  // Find and return data by key. Throws an exception (E_ITEM_NOT_FOUND)
  // if not found.
  const T& find(const char* Key) const;

  // Removes all items from the table (Doesn't deallocate table)
  void clear();

  // Allow the client to peer into the data
  OAHTStats GetStats() const;
  const OAHTSlot* GetTable() const;

private: // Some suggestions (You don't have to use any of this.)

  // Initialize the table after an allocation
  void init_table();

  // Expands the table when the load factor reaches a certain point
  // (greater than MaxLoadFactor) Grows the table by GrowthFactor,
  // making sure the new size is prime by calling GetClosestPrime
  void try_grow_table();

  void insert_inner(const char* Key, const T& Data, bool probe = true);

  template<typename S>
  struct SlotSearch {
    std::size_t index{0};
    S* slot{nullptr};
  };

  const SlotSearch<const OAHTSlot> find_slot(const char* Key) const;

  const SlotSearch<OAHTSlot> find_slot_mut(const char* Key);

  template<typename S>
  struct SlotProbe {
    SlotProbe(std::size_t index, S& slot);
    std::size_t index;
    S& slot;
  };

  const SlotProbe<const OAHTSlot> get_slot(std::size_t index, bool probe = true)
    const;

  const SlotProbe<OAHTSlot> get_slot_mut(std::size_t index, bool probe = true);

  std::size_t use_secondary_hash(const char* Key) const;

  const SlotProbe<const OAHTSlot> get_next_slot_with_index(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  ) const;

  const SlotProbe<OAHTSlot> get_next_slot_mut_with_index(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  );

  const OAHTSlot& get_next_slot(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  ) const;

  OAHTSlot& get_next_slot_mut(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  );

  void adjust_mark(std::size_t index);

  void adjust_pack(std::size_t index);

  void delete_slot(OAHTSlot& slot);

  OAHTConfig config{};

  OAHTSlot* slots{nullptr};

  HASHFUNC first_hash_function{nullptr};
  HASHFUNC second_hash_function{nullptr};
  FREEPROC delete_function{nullptr};

  mutable OAHTStats stats{};
};

  #ifndef OAHASHTABLE_CPP
    #include "OAHashTable.cpp"
  #endif

#endif

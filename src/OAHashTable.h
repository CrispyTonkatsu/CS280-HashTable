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

/**
 * @brief This is a Hash table for type trivially copiable T. It will function
 * according given to the provided OAConfig instance and keep track of its stats
 * in an OAStats instance. The hash table will own all the data that it
 * contains.
 */
template<typename T>
class OAHashTable {
public:

  /**
   * @brief Client-provided free function.
   */
  typedef void (*FREEPROC)(T);

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

  /**
   * @brief Slots that will hold the key/data pairs.
   */
  struct OAHTSlot {
    /**
     * @brief The 3 possible states the slot can be in:
     * - `OCCUPIED` means that this slot is currently filled with data.
     * - `UNOCCUPIED` means that this slot is currently not filled with data.
     * - `DELETED` means that this slot had its data deleted. Same as
     * `UNOCCUPIED` but its needed for lazy deletion.
     */
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

  /**
   * @brief Constructor for a Hash Table of type T
   *
   * @param Config The config that describes the table's behavior
   */
  OAHashTable(const OAHTConfig& Config);

  // TODO: Rule of 5

  /**
   * @brief Copy constructor for a Hash Table of type T
   *
   * @param rhs The table to copy from.
   */
  OAHashTable(const OAHashTable& rhs);

  /**
   * @brief Move constructor for a Hash Table of type T
   *
   * @param rhs The table being moved. Will be left in an invalid state.
   */
  OAHashTable(OAHashTable&& rhs);

  /**
   * @brief Copy assignment for a Hash Table of type T
   *
   * @param rhs The table to copy from.
   */
  OAHashTable& operator=(const OAHashTable& rhs);

  /**
   * @brief Move assignment for a Hash Table of type T
   *
   * @param rhs The table being moved. Will be left in an invalid state.
   */
  OAHashTable& operator=(OAHashTable&& rhs);

  /**
   * @brief Destructor for the table. It will call the deletion method for all
   * the occupied slots.
   */
  ~OAHashTable();

  /**
   * @brief Insert a key/data pair into table. Throws an exception if the
   * insertion is unsuccessful.
   *
   * @param Key The key to try insert in the table.
   * @param Data The data to insert into the table.
   */
  void insert(const char* Key, const T& Data);

  /**
   * @brief Delete an item by key. Throws an exception if the key doesn't exist.
   * Compacts the table by moving key/data pairs, if the deletion policy is
   * PACK.
   *
   * @param Key The key to erase if it's present.
   */
  void remove(const char* Key);

  /**
   * @brief Find and return data by key. Throws an exception (E_ITEM_NOT_FOUND)
   * if not found.
   *
   * @param Key The key to find if it's present.
   * @return The Data at Key
   */
  const T& find(const char* Key) const;

  /**
   * @brief Removes all items from the table (Doesn't deallocate table)
   */
  void clear();

  /**
   * @brief Returns the table's statistics (used for testing purposes).
   *
   * @return The table's stats.
   */
  OAHTStats GetStats() const;

  /**
   * @brief Returns the table's first element.
   *
   * @return The table's first slot.
   */
  const OAHTSlot* GetTable() const;

private:

  /**
   * @brief Initialize the table after an allocation
   *
   * @param reset_probes Whether to reset the probes for each slot.
   */
  void init_table(bool reset_probes = false);

  /**
   * @brief Expands the table when the load factor reaches a certain point
   * (greater than MaxLoadFactor) Grows the table by GrowthFactor,
   * making sure the new size is prime by calling GetClosestPrime
   */
  void try_grow_table();

  /**
   * @brief This is the real insert function, it's an abstraction used for
   * internal debugging and testing.
   *
   * @param Key The key to insert
   * @param Data The data to insert
   * @param probe Whether to count the accesses to the table as probes
   */
  void insert_inner(const char* Key, const T& Data, bool probe = true);

  /**
   * @brief This struct represents a search inside the table. If the S* is null
   * therefore it failed to locate the slot.
   */
  template<typename S>
  struct SlotSearch {
    std::size_t index{0};
    S* slot{nullptr};
  };

  /**
   * @brief This will try to find a slot in the table.
   *
   * @param Key The key to look for in the table.
   * @return A SlotSearch instance with the result of the search.
   */
  const SlotSearch<const OAHTSlot> find_slot(const char* Key) const;

  /**
   * @brief This will try to find a slot in the table.
   *
   * @param Key The key to look for in the table.
   * @return A SlotSearch instance with the result of the search.
   */
  const SlotSearch<OAHTSlot> find_slot_mut(const char* Key);

  /**
   * @brief This struct represents a probe inside the table. This is a
   * non-fallible invariant of the `SlotSearch` struct.
   */
  template<typename S>
  struct SlotProbe {
    /**
     * @brief The only way to construct a SlotProbe. This is needed due to it
     * storing a reference. Therefore not being default constructible.
     *
     * @param index The index of the slot.
     * @param slot A reference to a slot in the table.
     */
    SlotProbe(std::size_t index, S& slot);
    std::size_t index;
    S& slot;
  };

  /**
   * @brief This will get a slot given an index (it will map the value to the
   * table's range)
   *
   * @param index The index, it will be mapped to the table's range properly.
   * @param probe Whether this access should count as a probe.
   */
  const SlotProbe<const OAHTSlot> get_slot(std::size_t index, bool probe = true)
    const;

  /**
   * @brief This will get a slot given an index (it will map the value to the
   * table's range)
   *
   * @param index The index, it will be mapped to the table's range properly.
   * @param probe Whether this access should count as a probe.
   */
  const SlotProbe<OAHTSlot> get_slot_mut(std::size_t index, bool probe = true);

  /**
   * @brief This will use the secondary hash mapping the function parameters to
   * the required range of (1, TableSize - 1).
   *
   * @param Key The key to hash.
   * @return The output from the secondary hash function.
   */
  std::size_t use_secondary_hash(const char* Key) const;

  /**
   * @brief This will get the next slot using the proper stride (if required).
   *
   * @param index The index location to offset from.
   * @param offset The offset to apply.
   * @param probe Whether this access counts as a probe.
   *
   * @return A SlotProbe instance.
   */
  const SlotProbe<const OAHTSlot> get_next_slot_with_index(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  ) const;

  /**
   * @brief This will get the next slot using the proper stride (if required).
   *
   * @param index The index location to offset from.
   * @param offset The offset to apply.
   * @param probe Whether this access counts as a probe.
   *
   * @return A SlotProbe instance.
   */
  const SlotProbe<OAHTSlot> get_next_slot_mut_with_index(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  );

  /**
   * @brief This will get the next slot using the proper stride (if required).
   *
   * @param index The index location to offset from.
   * @param offset The offset to apply.
   * @param probe Whether this access counts as a probe.
   *
   * @return The next slot found with the given offset.
   */
  const OAHTSlot& get_next_slot(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  ) const;

  /**
   * @brief This will get the next slot using the proper stride (if required).
   *
   * @param index The index location to offset from.
   * @param offset The offset to apply.
   * @param probe Whether this access counts as a probe.
   *
   * @return The next slot found with the given offset.
   */
  OAHTSlot& get_next_slot_mut(
    const char* Key,
    std::size_t index,
    std::size_t offset,
    bool probe = true
  );

  /**
   * @brief To adjust the table with the deletion policy `MARK`.
   *
   * @param index The location to adjust from.
   */
  void adjust_mark(std::size_t index);

  /**
   * @brief To adjust the table with the deletion policy `PACK`.
   *
   * @param index The location to adjust from.
   */
  void adjust_pack(std::size_t index);

  /**
   * @brief Call the deletion function for the data in the slot and set the slot
   * to the right state.
   *
   * @param slot The slot to delete.
   */
  void delete_slot(OAHTSlot& slot);

  /**
   * @brief The table's configuration
   */
  OAHTConfig config{};

  /**
   * @brief The table's data storage
   */
  OAHTSlot* slots{nullptr};

  /**
   * @brief The first hash function to use, it should map to the range
   * (0,TableSize - 1)
   */
  HASHFUNC first_hash_function{nullptr};

  /**
   * @brief The second hash function to use, it should map to the range
   * (1,TableSize - 1)
   */
  HASHFUNC second_hash_function{nullptr};

  /**
   * @brief The function to call (if available) when deleting data in the table.
   */
  FREEPROC delete_function{nullptr};

  /**
   * @brief The table's stats.
   */
  mutable OAHTStats stats{};
};

  #ifndef OAHASHTABLE_CPP
    #include "OAHashTable.cpp"
  #endif

#endif

#ifndef __DATABASECOMMON_H
#define __DATABASECOMMON_H

#include "DarkCommon.h"
#include "integers.h"

namespace Opde {

/// Database event types
enum DatabaseChangeType {
    /// Loading a new database
    DBC_LOADING = 1,
    /// Saving a database
    DBC_SAVING,
    /// Dropping a database
    DBC_DROPPING
};

/*
 * FILE_TYPE [*] [*]
 * Magic-number of file.
 * Seems to be a bitmap representing what chunks might
 * appear in the file, but I can't find any specific associations.
 * Some archaic COWs I have used only the high-word.

 struct DarkDBChunkFILE_TYPE
 {
 uint32  type;
 };
 #define FILE_TYPE_VBR   0x00000500 // 0000 0000 0101
 #define FILE_TYPE_SAV   0x00011900 // 0001 0001 1001
 #define FILE_TYPE_MIS   0x00031900 // 0011 0001 1001
 #define FILE_TYPE_GAM   0x00040200 // 0100 0000 0010
 #define FILE_TYPE_COW   0x00071F00 // 0111 0001 1111
 #define FILE_TYPE_MASK_ARCHAIC  0xFFFF0000
// bit 8 - not on GAM
// bit 9 - only on GAM
// bit 10 - only on VBR
// bit 11 - only on MIS/SAV
// bit 12 - only on MIS/SAV
// bit 16 - MIS/SAV
// bit 17 - MIS
// bit 18 - GAM
*/

/** Database type encoding. Designed to be compatible with the FILE_TYPE tag for
 * later replacement. All this info derived directly from Telliamed's DarkUtils
 * FILE_TYPE description (Thanks again for all the work).
 *
 * [....:....|....:.A98|7654:3210|....:....]
 *
 * The high 3 bits (A98) contain historical data distribution guidelines: (A ==
 * ABSTRACT OBJECTS, 9==MISSON DATA, 8==CONCRETE OBJECTS)
 *
 * The Second lowest byte contains newer data separation guidelines (to be
 * combined with the high word):
 *
 * Concrete data : bit 1 (1). Active on databases that contain concrete object
 * tree Abstract data : bit 2 (2). Active on databases that contain gamesys
 * (abstract object tree and other abstract definitions - recipes) VBR      data
 * : bit 3 (4). Active only on VBR databases - that contain brush lists
 *
 */
enum DatabaseMask {
    /// Contains concrete objects (not necessarily object database!)
    DBM_CONCRETE = 0x0100,
    /// Contains Abstract objects
    DBM_ABSTRACT = 0x0200,
    /// Multibrush database - indicates multibrushes/minibrushes are present -
    /// VBR files have this (those have 0x0500 mask to be precise). Verified.
    DBM_MBRUSHES = 0x0400,
    /// Unknown. Present in cow only
    DBM_UNKNOWN1 = 0x0800,
    /// Contains concrete objects/links/properties, probably (or maybe AI state,
    /// sound propagation state, dunno...). A Mask because I can't yet separate
    /// which is what
    DBM_CONCRETE_OLP = 0x11900, // TODO: Decompose this one
    // High level bits (old encoding):
    /// Concrete object mask
    DBM_OBJTREE_CONCRETE = 0x010000,
    /// Mission data present mask
    DBM_MIS_DATA = 0x020000,
    /// Gamesys data present
    DBM_OBJTREE_GAMESYS = 0x040000,
    /// A complete - full database (e.g. VBR, also means all for DB_DROPPING)
    DBM_COMPLETE = 0x071F00,
    // File types follow
    /// GAM file type
    DBM_FILETYPE_GAM = 0x040200,
    /// VBR file type
    DBM_FILETYPE_VBR = 0x000500,
    /// SAV file type
    DBM_FILETYPE_SAV = 0x011900,
    /// MIS file type
    DBM_FILETYPE_MIS = 0x031900,
    /// COW file type
    DBM_FILETYPE_COW = 0x071F00,
};

/// The database change message
/*
  struct DatabaseChangeMsg {
  /// A change requested to happen
  DatabaseChangeType change;
  /// Type of the database distributed by this event
  DatabaseType dbtype;
  /// Type of the target database - valid options are DB_MISSION, DB_SAVEGAME
  and DB_COMPLETE DatabaseType dbtarget;
  /// The pointer to the database file to be used
  FileGroupPtr db;
  };
*/

/// Progress report of database loading. This is what the Progress Listener
/// get's every now and then to update the display
struct DatabaseProgressMsg {
    /// Completion status - 0.0-1.0
    float completed; // Only the coarse steps are included here. The fine steps
                     // are not
    /// Total coarse step count
    int totalCoarse;
    /// Current count of the coarse steps
    int currentCoarse;
    /// Overall count of the fine steps
    int overallFine; // Increased on every DatabaseService::fineStep (not
                     // cleared). Means the overall step count that happened

    /// Recalculates the completed
    void recalc() {
        if (totalCoarse > 0) {
            completed = static_cast<float>(currentCoarse) /
                        static_cast<float>(totalCoarse);
        }
    }

    void reset() {
        completed = 0;
        totalCoarse = 0;
        currentCoarse = 0;
        overallFine = 0;
    }
};

// Forward decl.
class DatabaseService;

/** Listener for database events. Pure abstract
 * @todo We need flags for various parts of the object system (compatible with
 * the mask 'FILE_TYPE' in tag file databases)
 */
class DatabaseListener {
    friend class DatabaseService;

protected:
    virtual void onDBLoad(const FileGroupPtr &db, uint32_t curmask) = 0;
    virtual void onDBSave(const FileGroupPtr &db, uint32_t tgtmask) = 0;
    virtual void onDBDrop(uint32_t dropmask) = 0;
};

} // namespace Opde

#endif /* __DATABASECOMMON_H */

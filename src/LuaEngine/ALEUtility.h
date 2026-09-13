/*
* Copyright (C) 2010 - 2025 Eluna Lua Engine <https://elunaluaengine.github.io/>
* This program is free software licensed under GPL version 3
* Please see the included DOCS/LICENSE.md for more information
*/

#ifndef _ALE_UTIL_H
#define _ALE_UTIL_H

#include <unordered_map>
#include <unordered_set>
#include <mutex>
#include <memory>
#include <string>
#include "Common.h"
#include "SharedDefines.h"
#include "ObjectGuid.h"
#include "Database/QueryResult.h"
#include "Log.h"
#include "Util.h"
#include <boost/filesystem.hpp>

typedef QueryResult ALEQuery;
#define GET_GUID                GetGUID
#define HIGHGUID_PLAYER         HighGuid::Player
#define HIGHGUID_UNIT           HighGuid::Unit
#define HIGHGUID_ITEM           HighGuid::Item
#define HIGHGUID_GAMEOBJECT     HighGuid::GameObject
#define HIGHGUID_PET            HighGuid::Pet
#define HIGHGUID_TRANSPORT      HighGuid::Transport
#define HIGHGUID_VEHICLE        HighGuid::Vehicle
#define HIGHGUID_CONTAINER      HighGuid::Container
#define HIGHGUID_DYNAMICOBJECT  HighGuid::DynamicObject
#define HIGHGUID_CORPSE         HighGuid::Corpse
#define HIGHGUID_MO_TRANSPORT   HighGuid::Mo_Transport
#define HIGHGUID_INSTANCE       HighGuid::Instance
#define HIGHGUID_GROUP          HighGuid::Group

#define ALE_LOG_INFO(...)     LOG_INFO("ALE", __VA_ARGS__);
#define ALE_LOG_ERROR(...)    LOG_ERROR("ALE", __VA_ARGS__);
#define ALE_LOG_DEBUG(...)    LOG_DEBUG("ALE", __VA_ARGS__);

#ifndef MAKE_NEW_GUID
#define MAKE_NEW_GUID(l, e, h)  ObjectGuid(h, e, l)
#endif
#ifndef GUID_ENPART
#define GUID_ENPART(guid)       ObjectGuid(guid).GetEntry()
#endif
#ifndef GUID_LOPART
#define GUID_LOPART(guid)       ObjectGuid(guid).GetCounter()
#endif
#ifndef GUID_HIPART
#define GUID_HIPART(guid)       ObjectGuid(guid).GetHigh()
#endif

class Unit;
class WorldObject;
struct FactionTemplateEntry;

namespace ALEUtil
{
    uint32 GetCurrTime();

    uint32 GetTimeDiff(uint32 oldMSTime);

    class ObjectGUIDCheck
    {
    public:
        ObjectGUIDCheck(ObjectGuid guid);
        bool operator()(WorldObject* object);

        ObjectGuid _guid;
    };

    // Binary predicate to sort WorldObjects based on the distance to a reference WorldObject
    class ObjectDistanceOrderPred
    {
    public:
        ObjectDistanceOrderPred(WorldObject const* pRefObj, bool ascending = true);
        bool operator()(WorldObject const* pLeft, WorldObject const* pRight) const;

        WorldObject const* m_refObj;
        const bool m_ascending;
    };

    // Doesn't get self
    class WorldObjectInRangeCheck
    {
    public:
        WorldObjectInRangeCheck(bool nearest, WorldObject const* obj, float range,
            uint16 typeMask = 0, uint32 entry = 0, uint32 hostile = 0, uint32 dead = 0);
        WorldObject const& GetFocusObject() const;
        bool operator()(WorldObject* u);

        WorldObject const* const i_obj;
        Unit const* i_obj_unit;
        FactionTemplateEntry const* i_obj_fact;
        uint32 const i_hostile; // 0 both, 1 hostile, 2 friendly
        uint32 const i_entry;
        float i_range;
        uint16 const i_typeMask;
        uint32 const i_dead; // 0 both, 1 alive, 2 dead
        bool const i_nearest;
    };

    /*
     * Usage:
     * Inherit this class, then when needing lock, use
     * Guard guard(GetLock());
     *
     * The lock is automatically released at end of scope
     */
    class Lockable
    {
    public:
        typedef std::mutex LockType;
        typedef std::lock_guard<LockType> Guard;

        LockType& GetLock() { return _lock; }

    private:
        LockType _lock;
    };

    /*
     * Encodes `data` in Base-64 and store the result in `output`.
     */
    void EncodeData(const unsigned char* data, size_t input_length, std::string& output);

    /*
     * Decodes `data` from Base-64 and returns a pointer to the result, or `NULL` on error.
     *
     * The returned result buffer must be `delete[]`ed by the caller.
     */
    unsigned char* DecodeData(const char* data, size_t *output_length);
};

namespace ALEPathUtil
{
    // Convert boost::filesystem::path to UTF-8 encoded std::string
    // On Windows: converts from wide string (UTF-16) using WStrToUtf8
    // On Linux/macOS: uses native UTF-8 generic_string directly
    inline std::string ToUtf8String(const boost::filesystem::path& path)
    {
#if AC_PLATFORM == AC_PLATFORM_WINDOWS
        std::wstring wpath = path.wstring();
        std::string result;
        if (WStrToUtf8(wpath, result))
            return result;
        // Fallback: if conversion fails, use narrow string (may have ANSI encoding issues, but won't crash)
        return path.string();
#else
        return path.generic_string();
#endif
    }

    // Convert UTF-8 encoded std::string to boost::filesystem::path
    // On Windows: converts from UTF-8 to wide string (UTF-16) first
    // On Linux/macOS: uses UTF-8 string directly
    inline boost::filesystem::path FromUtf8String(const std::string& utf8Path)
    {
#if AC_PLATFORM == AC_PLATFORM_WINDOWS
        std::wstring wpath;
        if (Utf8toWStr(utf8Path, wpath))
            return boost::filesystem::path(wpath);
        // Fallback: if conversion fails, use narrow string
        return boost::filesystem::path(utf8Path);
#else
        return boost::filesystem::path(utf8Path);
#endif
    }

    // Get filename as UTF-8 string from a directory_iterator entry
    inline std::string GetFilenameUtf8(const boost::filesystem::directory_iterator& dir_iter)
    {
        return ToUtf8String(dir_iter->path().filename());
    }

    // Get full path as UTF-8 string from a directory_iterator entry
    inline std::string GetFullpathUtf8(const boost::filesystem::directory_iterator& dir_iter)
    {
        return ToUtf8String(dir_iter->path());
    }
}

#endif

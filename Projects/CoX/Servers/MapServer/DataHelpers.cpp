/*
 * SEGS - Super Entity Game Server
 * http://www.segs.io/
 * Copyright (c) 2006 - 2018 SEGS Team (see AUTHORS.md)
 * This software is licensed under the terms of the 3-clause BSD License. See LICENSE.md for details.
 */

/*!
 * @addtogroup MapServer Projects/CoX/Servers/MapServer
 * @{
 */

#include "DataHelpers.h"

#include "MapServer.h"
#include "MapServerData.h"
#include "MapInstance.h"
#include "GameData/playerdata_definitions.h"
#include "NetStructures/Character.h"
#include "NetStructures/Team.h"
#include "NetStructures/LFG.h"
#include "Events/EmailHeaders.h"
#include "Events/EmailRead.h"
#include "Logging.h"

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

/*
 * Entity Methods
 */
// Getters
uint32_t    getIdx(const Entity &e) { return e.m_idx; }
uint32_t    getDbId(const Entity &e) { return e.m_db_id; }
uint32_t    getAccessLevel(const Entity &e) { return e.m_entity_data.m_access_level; }
uint32_t    getTargetIdx(const Entity &e) { return e.m_target_idx; }
uint32_t    getAssistTargetIdx(const Entity &e) { return e.m_assist_target_idx; }
glm::vec3   getSpeed(const Entity &e) { return e.m_spd; }
float       getBackupSpd(const Entity &e) { return e.m_backup_spd; }
float       getJumpHeight(const Entity &e) { return e.m_jump_height; }
uint8_t     getUpdateId(const Entity &e) { return e.m_update_id; }

// Setters
void    setDbId(Entity &e, uint8_t val) { e.m_char->m_db_id = val; e.m_db_id = val; }
void    setMapIdx(Entity &e, uint32_t val) { e.m_entity_data.m_map_idx = val; }
void    setSpeed(Entity &e, float v1, float v2, float v3) { e.m_spd = {v1,v2,v3}; }
void    setBackupSpd(Entity &e, float val) { e.m_backup_spd = val; }
void    setJumpHeight(Entity &e, float val) { e.m_jump_height = val; }
void    setUpdateID(Entity &e, uint8_t val) { e.m_update_id = val;}

void    setTeamID(Entity &e, uint8_t team_id)
{
    if(team_id == 0)
    {
        e.m_has_team            = false;
        delete e.m_team;
        e.m_team = nullptr;
    }
    else
        e.m_has_team            = true;

    if(!e.m_team)
        return;

    qDebug().noquote() << "Team Info:"
                       << "\n  Has Team:" << e.m_has_team
                       << "\n  ID:" << e.m_team->m_team_idx
                       << "\n  Size:" << e.m_team->m_team_members.size()
                       << "\n  Members:" << e.m_team->m_team_members.data();
}

void    setSuperGroup(Entity &e, int sg_id, QString sg_name, uint32_t sg_rank)
{
    // TODO: provide method for updating SuperGroup Colors
    if(sg_id == 0)
    {
        e.m_has_supergroup          = false;
        e.m_supergroup.m_SG_id      = 0;
        e.m_supergroup.m_SG_name    = "";
        e.m_supergroup.m_SG_color1  = 0x996633FF;
        e.m_supergroup.m_SG_color2  = 0x336699FF;
        e.m_supergroup.m_SG_rank    = 0;
    }
    else
    {
        e.m_has_supergroup          = true;
        e.m_supergroup.m_SG_id      = sg_id;
        e.m_supergroup.m_SG_name    = sg_name;
        e.m_supergroup.m_SG_color1  = 0xAA3366FF;
        e.m_supergroup.m_SG_color2  = 0x66AA33FF;
        e.m_supergroup.m_SG_rank    = sg_rank;
    }
    qDebug().noquote() << "SG Info:"
             << "\n  Has Team:" << e.m_has_supergroup
             << "\n  ID:" << e.m_supergroup.m_SG_id
             << "\n  Name:" << e.m_supergroup.m_SG_name
             << "\n  Color1:" << e.m_supergroup.m_SG_color1
             << "\n  Color2:" << e.m_supergroup.m_SG_color2
             << "\n  Rank:" << e.m_supergroup.m_SG_rank;
}

void setTarget(Entity &e, uint32_t target_idx)
{
    e.m_target_idx = target_idx;
    setAssistTarget(e);
}

void setAssistTarget(Entity &e)
{
    if(getTargetIdx(e) == 0)
    {
        e.m_assist_target_idx = 0;
        return;
    }

    Entity *target_ent = getEntity(e.m_client, getTargetIdx(e));
    if(target_ent == nullptr)
    {
        e.m_assist_target_idx = 0;
        return;
    }

    // TODO: are there any entity types that are invalid assist targets?
    e.m_assist_target_idx = getTargetIdx(*target_ent);

    qCDebug(logTarget) << "Assist Target is:" << getAssistTargetIdx(e);
}

// For live debugging
void    setu1(Entity &e, int val) { e.u1 = val; }

// Toggles
void    toggleFlying(Entity &e) { e.m_is_flying = !e.m_is_flying; }
void    toggleFalling(Entity &e) { e.m_is_falling = !e.m_is_falling; }
void    toggleJumping(Entity &e) { e.m_is_jumping = !e.m_is_jumping; }
void    toggleSliding(Entity &e) { e.m_is_sliding = !e.m_is_sliding; }

void toggleStunned(Entity &e)
{
    e.m_is_stunned = !e.m_is_stunned;
    // TODO: toggle stunned FX above head
}

void toggleJumppack(Entity &e)
{
    e.m_has_jumppack = !e.m_has_jumppack;
    // TODO: toggle costume part for jetpack back item.
}

void    toggleControlsDisabled(Entity &e) { e.m_controls_disabled = !e.m_controls_disabled; }
void    toggleFullUpdate(Entity &e) { e.m_full_update = !e.m_full_update; }
void    toggleControlId(Entity &e) { e.m_has_control_id = !e.m_has_control_id; }
void    toggleExtraInfo(Entity &e) { e.m_extra_info = !e.m_extra_info; }
void    toggleMoveInstantly(Entity &e) { e.m_move_instantly = !e.m_move_instantly; }

// Misc Methods
void charUpdateDB(Entity *e)
{
    markEntityForDbStore(e,DbStoreFlags::Full);
}

int getEntityOriginIndex(bool is_player, const QString &origin_name)
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllOrigins &origins_to_search(is_player ? data.m_player_origins : data.m_other_origins);

    int idx = 0;
    for(const Parse_Origin &orig : origins_to_search)
    {
        if(orig.Name.compare(origin_name,Qt::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    qWarning() << "Failed to locate origin index for"<<origin_name;
    return 0;
}

int getEntityClassIndex(bool is_player, const QString &class_name)
{
    const MapServerData &data(g_GlobalMapServer->runtimeData());
    const Parse_AllCharClasses &classes_to_search(is_player ? data.m_player_classes : data.m_other_classes);

    int idx = 0;
    for(const CharClass_Data &classdata : classes_to_search)
    {
        if(classdata.m_Name.compare(class_name,Qt::CaseInsensitive)==0)
            return idx;
        idx++;
    }
    qWarning() << "Failed to locate class index for"<<class_name;
    return 0;

}

// Poll EntityManager to return Entity by Name or IDX
Entity * getEntity(MapClientSession *src, const QString &name)
{
    MapInstance *mi = src->m_current_map;
    EntityManager &em(mi->m_entities);
    QString errormsg;

    // Iterate through all active entities and return entity by name
    for (Entity* pEnt : em.m_live_entlist)
    {
        if (pEnt->name() == name)
            return pEnt;
    }

    errormsg = "Entity " + name + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    sendInfoMessage(MessageChannel::USER_ERROR, errormsg, src);
    return nullptr;
}

Entity * getEntity(MapClientSession *src, uint32_t idx)
{
    MapInstance *mi = src->m_current_map;
    EntityManager &em(mi->m_entities);
    QString errormsg;

    if(idx!=0) // Entity idx 0 is special case, so we can't return it
    {
        // Iterate through all active entities and return entity by idx
        for (Entity* pEnt : em.m_live_entlist)
        {
            if (pEnt->m_idx == idx)
                return pEnt;
        }
    }
    errormsg = "Entity " + QString::number(idx) + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    sendInfoMessage(MessageChannel::USER_ERROR, errormsg, src);
    return nullptr;
}

Entity *getEntityByDBID(MapClientSession *src, uint32_t db_id)
{
    MapInstance *  mi = src->m_current_map;
    EntityManager &em(mi->m_entities);
    QString        errormsg;

    if (db_id == 0)
    {
        errormsg = "Entity " + QString::number(db_id) + " does not exist in the database.";
        qWarning() << errormsg;
        sendInfoMessage(MessageChannel::USER_ERROR, errormsg, src);
        return nullptr;
    }
    // TODO: Iterate through all entities in Database and return entity by db_id
    for (Entity *pEnt : em.m_live_entlist)
    {
        if (pEnt->m_db_id == db_id)
            return pEnt;
    }

    errormsg = "Entity with db_id " + QString::number(db_id) + " does not exist, or is not currently online.";
    qWarning() << errormsg;
    sendInfoMessage(MessageChannel::USER_ERROR, errormsg, src);
    return nullptr;
}

void sendServerMOTD(MapClientSession *tgt)
{
    qDebug().noquote() << "Sending Server MOTD to" << tgt->m_ent->m_char->getName();

    QString fileName("scripts/motd.smlx");
    QFile file(fileName);
    if(file.exists() && file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QString contents(file.readAll());
        StandardDialogCmd *dlg = new StandardDialogCmd(contents);
        tgt->addCommandToSendNextUpdate(std::unique_ptr<StandardDialogCmd>(dlg));
    }
    else {
        QString errormsg = "Failed to load MOTD file. \'" + file.fileName() + "\' not found.";
        qDebug() << errormsg;
        sendInfoMessage(MessageChannel::DEBUG_INFO, errormsg, tgt);
    }
}

void sendEmailHeaders(Entity *e)
{
    if(!e->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }
    MapClientSession *src = e->m_client;

    EmailHeaders *header = new EmailHeaders(152, "TestSender ", "TEST", 576956720);
    src->addCommandToSendNextUpdate(std::unique_ptr<EmailHeaders>(header));
}

void readEmailMessage(Entity *e, const int id){
    if(!e->m_client)
    {
        qWarning() << "m_client does not yet exist!";
        return;
    }
    MapClientSession *src = e->m_client;

    EmailRead *msg = new EmailRead(id, "https://youtu.be/PsCKnxe8hGY\\nhttps://youtu.be/dQw4w9WgXcQ", "TestSender");
    src->addCommandToSendNextUpdate(std::unique_ptr<EmailRead>(msg));
}

/*
 * Character Methods
 */
// TODO: get titles from texts/English/titles_def
static const QStringList g_generic_titles =
{
    "NULL",
    "Awesome",
    "Bold",
    "Courageous",
    "Daring",
    "Extraordinary",
    "Famous",
    "Gallant",
    "Heroic",
    "Incomparable",
    "Legendary",
    "Magnificent",
    "Outstanding",
    "Powerful",
    "Remarkable",
    "Startling",
    "Terrific",
    "Ultimate",
    "Valiant",
    "Wonderful",
};

// TODO: get titles from texts/English/titles_def
static const QStringList g_origin_titles =
{
    "NULL",
    "Adept",
    "Bright",
    "Curious",
    "Deductiv",
    "Exceptional",
    "Far Seeing",
    "Glorious",
    "Honorable",
    "Indescribable",
    "Lucky",
    "Majestic",
    "Otherworldly",
    "Phenomenal",
    "Redoubtable",
    "Stupendous",
    "Thoughtful",
    "Unearthly",
    "Venturous",
    "Watchful",
};

// Getter
uint32_t            getLevel(const Character &c) { return c.m_char_data.m_level; }
uint32_t            getCombatLevel(const Character &c) { return c.m_char_data.m_combat_level; }
float               getHP(const Character &c) { return c.m_char_data.m_current_attribs.m_HitPoints; }
float               getEnd(const Character &c) { return c.m_char_data.m_current_attribs.m_Endurance; }
uint64_t            getLastCostumeId(const Character &c) { return c.m_char_data.m_last_costume_id; }
const QString &     getOrigin(const Character &c) { return c.m_char_data.m_origin_name; }
const QString &     getClass(const Character &c) { return c.m_char_data.m_class_name; }
uint32_t            getXP(const Character &c) { return c.m_char_data.m_experience_points; }
uint32_t            getDebt(const Character &c) { return c.m_char_data.m_experience_debt; }
uint32_t            getPatrolXP(const Character &c) { return c.m_char_data.m_experience_patrol; }
const QString &     getGenericTitle(const Character &c) { return c.m_char_data.m_titles[0]; }
const QString &     getGenericTitle(uint32_t val) { return g_generic_titles.at(val); }
const QString &     getOriginTitle(const Character &c) { return c.m_char_data.m_titles[1]; }
const QString &     getOriginTitle(uint32_t val) { return g_origin_titles.at(val); }
const QString &     getSpecialTitle(const Character &c) { return c.m_char_data.m_titles[2]; }
uint32_t            getInf(const Character &c) { return c.m_char_data.m_influence; }
const QString &     getDescription(const Character &c) { return c.m_char_data.m_character_description ; }
const QString &     getBattleCry(const Character &c) { return c.m_char_data.m_battle_cry; }
const QString &     getAlignment(const Character &c) { return c.m_char_data.m_alignment; }

static const std::vector<MapData> g_defined_map_datas =
{
    // City_Zones
    {0, "City_00_01", "maps/City_Zones/City_00_01/City_00_01.txt", "Outbreak"},
    {1, "City_01_01", "maps/City_Zones/City_01_01/City_01_01.txt", "Atlas Park"},
    {2, "City_01_02", "maps/City_Zones/City_01_02/City_01_02.txt", "King's Row"},
    {3, "City_01_03", "maps/City_Zones/City_01_03/City_01_03.txt", "Galaxy City"},
    {4, "City_02_01", "maps/City_Zones/City_02_01/City_02_01.txt", "Steel Canyon"},
    {5, "City_02_02", "maps/City_Zones/City_02_02/City_02_02.txt", "Skyway City"},
    {6, "City_03_01", "maps/City_Zones/City_03_01/City_03_01.txt", "Talos Island"},
    {7, "City_03_02", "maps/City_Zones/City_03_02/City_03_02.txt", "Independence Port"},
    {8, "City_04_01", "maps/City_Zones/City_04_01/City_04_01.txt", "Founders' Falls"},
    {9, "City_04_02", "maps/City_Zones/City_04_02/City_04_02.txt", "Brickstown"},
    {10, "City_05_01", "maps/City_Zones/City_05_01/City_05_01.txt", "Peregrine Island"},

    // Hazards
    {11, "Hazard_01_01", "maps/City_Zones/Hazard_01_01/Hazard_01_01.txt", "Perez Park"},
    {12, "Hazard_02_01", "maps/City_Zones/Hazard_02_01/Hazard_02_01.txt", "Boomtown"},
    {13, "Hazard_03_01", "maps/City_Zones/Hazard_03_01/Hazard_03_01.txt", "Dark Astoria"},
    {14, "Hazard_04_01", "maps/City_Zones/Hazard_04_01/Hazard_04_01.txt", "Crey's Folly"},
    {15, "Hazard_04_02", "maps/City_Zones/Hazard_04_02/Hazard_04_02.txt", "Enviro Nightmare"},
    {16, "Hazard_05_01", "maps/City_Zones/Hazard_05_01/Hazard_05_01.txt", "Elysium"},

    // Trials
    {17, "Trial_01_01", "maps/City_Zones/Trial_01_01/Trial_01_01.txt", "Abandoned Sewer Network"},
    {18, "Trial_01_02", "maps/City_Zones/Trial_01_02/Trial_01_02.txt", "Sewer Network"},
    {19, "Trial_02_01", "maps/City_Zones/Trial_02_01/Trial_02_01.txt", "Faultline"},
    {20, "Trial_03_01", "maps/City_Zones/Trial_03_01/Trial_03_01.txt", "Terra Volta"},
    {21, "Trial_04_01", "maps/City_Zones/Trial_04_01/Trial_04_01.txt", "Eden"},
    {22, "Trial_04_02", "maps/City_Zones/Trial_04_02/Trial_04_02.txt", "The Hive"},
    {23, "Trial_05_01", "maps/City_Zones/Trial_05_01/Trial_05_01.txt", "Rikti Crash Site"}
};

MapData getMapData(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
    {
        if(map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data;
    }

    // If no map is found, log a warning and return Outbreak's data.
    qWarning() << "No match for \"" << map_name << "\" in g_defined_map_datas."
               << "Returning Outbreak's map data as default...";
    return g_defined_map_datas[0];
}

uint32_t getMapIndex(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
    {
        if (map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_map_idx;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching map name in g_defined_map_datas to sent map name."
               << "Returning Outbreak's map index as default...";

    // defaulting to Outbreak's map name
    return 0;
}

const QString getDisplayMapName(const QString &map_name)
{
    for (const auto &map_data : g_defined_map_datas)
    {
        if (map_name.contains(map_data.m_map_name, Qt::CaseInsensitive))
            return map_data.m_display_map_name;
    }

    // log a warning because this part of the code is called when things went wrong
    qWarning() << "No matching map name in g_defined_map_datas to sent map name."
               << "Returning Outbreak's display map name as default...";

    // defaulting to Outbreak's map name
    return g_defined_map_datas[0].m_display_map_name;
}

const QString getDisplayMapName(size_t index)
{
    // Since index is unsigned, it cannot be negative.
    // Thus, no need to check for index < 0.
    if(index >= g_defined_map_datas.size())
    {
        qWarning() << "Sought map index was out of range."
                   << "Returning Outbreak's display map name as default...";
        index = 0;
    }
    return g_defined_map_datas[index].m_display_map_name;
}

const QString getMapName(size_t index)
{
    // Since index is unsigned, it cannot be negative.
    // Thus, no need to check for index < 0.
    if(index >= g_defined_map_datas.size())
    {
        qWarning() << "Sought map index was out of range."
                   << "Returning Outbreak's map name as default...";
        index = 0;
    }
    return g_defined_map_datas[index].m_map_name;
}

const QString getMapPath(const EntityData &ed)
{
    return getMapPath(ed.m_map_idx);
}

const QString getMapPath(size_t index)
{
    if(index >= g_defined_map_datas.size()){
        qWarning() << "Sought map index was out of range."
                   << "Returning Outbreak's map path as default...";
        index = 0;
    }
    return g_defined_map_datas[index].m_map_path;
}

const QString getEntityDisplayMapName(const EntityData &ed)
{
    return getDisplayMapName(ed.m_map_idx);
}

const QString getFriendDisplayMapName(const Friend &f)
{
    if (!f.m_online_status)
        return "OFFLINE";
    return getDisplayMapName(f.m_map_idx);
}

// Setters
void setLevel(Character &c, uint32_t val)
{
    if(val>50)
        val = 50;
    c.m_char_data.m_level = val - 1; // client stores lvl arrays starting at 0
    c.finalizeLevel();
}

void setCombatLevel(Character &c, uint32_t val)
{
    if(val>50)
        val = 50;
    c.m_char_data.m_combat_level = val;
}

void setHP(Character &c, float val)
{
    c.m_char_data.m_current_attribs.m_HitPoints = std::max(0.0f, std::min(val,c.m_max_attribs.m_HitPoints));
}

void setEnd(Character &c, float val)
{
    c.m_char_data.m_current_attribs.m_Endurance = std::max(0.0f, std::min(val,c.m_max_attribs.m_Endurance));
}

void    setLastCostumeId(Character &c, uint64_t val) { c.m_char_data.m_last_costume_id = val; }

void setXP(Character &c, uint32_t val)
{
    c.m_char_data.m_experience_points = val;
    for (auto const &lvl : c.m_other_attribs.m_ExperienceRequired)
    {
        if (val >= lvl && val < lvl + 1)
        {
            setLevel(c, lvl);
            // TODO: set max attribs based upon level.
        }
    }
}

void setDebt(Character &c, uint32_t val) { c.m_char_data.m_experience_debt = val; }

void setTitles(Character &c, bool prefix, QString generic, QString origin, QString special)
{
    // if "NULL", clear string
    if(generic=="NULL")
        generic.clear();
    if(origin=="NULL")
        origin.clear();
    if(special=="NULL")
        special.clear();

    c.m_char_data.m_has_titles = prefix || !generic.isEmpty() || !origin.isEmpty() || !special.isEmpty();
    if(!c.m_char_data.m_has_titles)
      return;

    c.m_char_data.m_has_the_prefix = prefix;
    c.m_char_data.m_titles[0] = generic;
    c.m_char_data.m_titles[1] = origin;
    c.m_char_data.m_titles[2] = special;
}

void setInf(Character &c, uint32_t val) { c.m_char_data.m_influence = val; }
void setDescription(Character &c, QString val) { c.m_char_data.m_character_description = val; }
void setBattleCry(Character &c, QString val) { c.m_char_data.m_battle_cry = val; }

// Toggles
void toggleAFK(Character &c, const QString &msg)
{
    c.m_char_data.m_afk = !c.m_char_data.m_afk;
    if(c.m_char_data.m_afk)
        c.m_char_data.m_afk_msg = msg;
}

void toggleTeamBuffs(PlayerData &c) { c.m_gui.m_team_buffs = !c.m_gui.m_team_buffs; }

/*
 * Looking For Group
 */
void toggleLFG(Entity &e)
{
    CharacterData *cd = &e.m_char->m_char_data;

    if(e.m_has_team)
    {
        QString errormsg = "You're already on a team! You cannot toggle LFG.";
        sendInfoMessage(MessageChannel::USER_ERROR, errormsg, e.m_client);
        errormsg = e.name() + "is already on a team and cannot toggle LFG.";
        qCDebug(logTeams) << errormsg;
    }

    if(cd->m_lfg)
        removeLFG(e);
    else
    {
        addLFG(e);
        sendTeamLooking(&e);
    }
}

/*
 * getMapServerData Wrapper to provide access to NetStructures
 */
MapServerData *getMapServerData()
{
    return &g_GlobalMapServer->runtimeData();
}

/*
 * sendInfoMessage wrapper to provide access to NetStructures
 */
void messageOutput(MessageChannel ch, QString &msg, Entity &tgt)
{
    sendInfoMessage(ch, msg, tgt.m_client);
}

/*
 * SendUpdate Wrappers to provide access to NetStructures
 */
void sendClientState(Entity *tgt, ClientStates client_state)
{
    qCDebug(logSlashCommand) << "Sending ClientState:" << tgt->m_idx << QString::number(client_state);
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<SetClientState>(new SetClientState(client_state)));
}

void showMapXferList(Entity *ent, bool has_location, glm::vec3 &location, QString &name)
{
    qCDebug(logSlashCommand) << "Showing MapXferList:" << ent->m_idx << name;
    ent->m_client->addCommandToSendNextUpdate(std::unique_ptr<MapXferList>(new MapXferList(has_location, location, name)));
}

void sendFloatingInfo(Entity *tgt, QString &msg, FloatingInfoStyle style, float delay)
{
    qCDebug(logSlashCommand) << "Sending FloatingInfo:" << tgt->m_idx << msg;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<FloatingInfo>(new FloatingInfo(tgt->m_idx, msg, style, delay)));
}

void sendFloatingNumbers(Entity *src, uint32_t tgt_idx, int32_t amount)
{
    qCDebug(logSlashCommand, "Sending %d FloatingNumbers from %d to %d", amount, src->m_idx, tgt_idx);
    src->m_client->addCommandToSendNextUpdate(std::unique_ptr<FloatingDamage>(new FloatingDamage(src->m_idx, tgt_idx, amount)));
}

void sendLevelUp(Entity *tgt)
{
    //qCDebug(logSlashCommand) << "Sending LevelUp:" << tgt->m_idx;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<LevelUp>(new LevelUp()));
}

void sendEnhanceCombineResponse(Entity *tgt, bool success, bool destroy)
{
    //qCDebug(logSlashCommand) << "Sending CombineEnhanceResponse:" << tgt->m_idx;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<CombineEnhanceResponse>(new CombineEnhanceResponse(success, destroy)));
}

void sendChangeTitle(Entity *tgt, bool select_origin)
{
    //qCDebug(logSlashCommand) << "Sending ChangeTitle Dialog:" << tgt->m_idx << "select_origin:" << select_origin;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<ChangeTitle>(new ChangeTitle(select_origin)));
}

void sendTrayAdd(Entity *tgt, uint32_t pset_idx, uint32_t pow_idx)
{
    qCDebug(logSlashCommand) << "Sending TrayAdd:" << tgt->m_idx << pset_idx << pow_idx;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<TrayAdd>(new TrayAdd(pset_idx, pow_idx)));
}

void sendFriendsListUpdate(Entity *src, FriendsList *friends_list)
{
    qCDebug(logFriends) << "Sending FriendsList Update.";
    src->m_client->addCommandToSendNextUpdate(std::unique_ptr<FriendsListUpdate>(new FriendsListUpdate(friends_list)));
}

void sendSidekickOffer(Entity *tgt, uint32_t src_db_id)
{
    qCDebug(logTeams) << "Sending Sidekick Offer" << tgt->name() << "from" << src_db_id;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<SidekickOffer>(new SidekickOffer(src_db_id)));
}

void sendTeamLooking(Entity *tgt)
{
    std::vector<LFGMember> list = g_lfg_list;

    qCDebug(logLFG) << "Sending Team Looking to" << tgt->name();
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<TeamLooking>(new TeamLooking(list)));
}

void sendTeamOffer(Entity *src, Entity *tgt)
{
    QString name        = src->name();
    uint32_t db_id      = tgt->m_db_id;
    TeamOfferType type  = NoMission;

    // Check for mission, send appropriate TeamOfferType
    if(src->m_has_team && src->m_team != nullptr)
        if(src->m_team->m_team_has_mission)
            type = WithMission; // TODO: Check for invalid missions to send `LeaveMission` instead

    qCDebug(logTeams) << "Sending Teamup Offer" << db_id << name << type;
    tgt->m_client->addCommandToSendNextUpdate(std::unique_ptr<TeamOffer>(new TeamOffer(db_id, name, type)));
}

//! @}

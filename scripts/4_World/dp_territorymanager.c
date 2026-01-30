class TM_TerritoryOwnership
{
    string ownerId;
    Object flagObj;
    void TM_TerritoryOwnership() {}
}

class DP_TerritoryManager
{
    protected static ref DP_TerritoryManager s_Instance;
    protected ref array<ref TM_TerritoryOwnership> m_Ownerships;
    
    Object m_ClientPendingTarget;
    string m_LastReceivedOwnerID;

    void DP_TerritoryManager()
    {
        m_Ownerships = new array<ref TM_TerritoryOwnership>();
    }

    static DP_TerritoryManager TM_GetInstance()
    {
        if (!s_Instance) s_Instance = new DP_TerritoryManager();
        return s_Instance;
    }

    Object TM_GetNearestFlag(vector position, float radius)
    {
        float minDistSq = radius * radius;
        for (int i = 0; i < m_Ownerships.Count(); ++i)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (!to || !to.flagObj) continue;
            
            float searchR = radius; 
            TerritoryFlag flag = TerritoryFlag.Cast(to.flagObj);
            if (flag) searchR = flag.GetCurrentRadius();
            
            float distSq = vector.DistanceSq(position, to.flagObj.GetPosition());
            if (distSq < (searchR * searchR)) return to.flagObj;
        }
        return null;
    }

    bool TM_IsTerritoryNearby(vector position, float safeDistance)
    {
        float safeDistSq = safeDistance * safeDistance;
        for (int i = 0; i < m_Ownerships.Count(); ++i)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (!to || !to.flagObj) continue;
            float distSq = vector.DistanceSq(position, to.flagObj.GetPosition());
            if (distSq < safeDistSq) return true; 
        }
        return false;
    }

    bool TM_HasTerritory(string ownerId)
    {
        if (!ownerId) return false;
        for (int i = 0; i < m_Ownerships.Count(); ++i)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (to && to.ownerId == ownerId) return true;
        }
        return false;
    }
    
    bool IsPlacedObject(Object obj)
    {
        if (!obj) return false;
        EntityAI entity = EntityAI.Cast(obj);
        if (!entity) return false;
        if (entity.IsHologram()) return false;
        if (entity.GetHierarchyParent()) return false; 
        TentBase tent = TentBase.Cast(obj);
        if (tent) { if (tent.GetState() == 0) return false; }
        return true;
    }

    bool TM_CheckLimits(PlayerBase player, string itemClassname)
    {
        if (!player) return false;
        float maxR = DP_TerritoryConfig.Get().GetMaxPossibleRadius();
        Object flagObj = TM_GetNearestFlag(player.GetPosition(), maxR);
        if (!flagObj) return true; 
        
        TerritoryFlag flag = TerritoryFlag.Cast(flagObj);
        if (!flag) return true;

        int category = DP_TerritoryConfig.Get().GetItemCategory(itemClassname);
        if (category == 0) return true; 
        
        int lvl = flag.GetTerritoryLevel();
        DP_LevelDefinition levelDef = DP_TerritoryConfig.Get().GetLevelConfig(lvl);
        if (!levelDef) return true;

        float currentR = flag.GetCurrentRadius();
        array<Object> objects = new array<Object>;
        array<CargoBase> proxyCargos = new array<CargoBase>;
        GetGame().GetObjectsAtPosition(flag.GetPosition(), currentR, objects, proxyCargos);
        
        int currentCount = 0;
        int maxLimit = 999999;
        if (category == 1) maxLimit = levelDef.MaxStructures;
        if (category == 2) maxLimit = levelDef.MaxTents;
        if (category == 3) maxLimit = levelDef.MaxFurniture;

        float bonusSlots = 0;
        if (player.GetTerjeSkills()) 
        {
            player.GetTerjeSkills().GetPerkValue("Skill_Architect", "Perk_Foreman", bonusSlots);
        }
        maxLimit += (int)bonusSlots;

        for (int i = 0; i < objects.Count(); i++)
        {
            Object obj = objects.Get(i);
            if (!obj) continue;
            if (!IsPlacedObject(obj)) continue;
            if (DP_TerritoryConfig.Get().GetItemCategory(obj.GetType()) == category) currentCount++;
        }
        
        if ((currentCount + 1) > maxLimit)
        {
             player.MessageImportant("⛔ Лимит (" + lvl + " ур): " + maxLimit + " (Перк +" + (int)bonusSlots + ")");
             return false;
        }
        return true;
    }

    bool TM_RegisterOwner_Unique(string ownerId, Object flagObj)
    {
        if (!ownerId || !flagObj) return false;
        TM_UnregisterByOwnerId(ownerId);
        TM_TerritoryOwnership to = new TM_TerritoryOwnership();
        to.ownerId = ownerId;
        to.flagObj = flagObj;
        m_Ownerships.Insert(to);
        return true;
    }

    void TM_UnregisterByOwnerId(string ownerId)
    {
        if (!ownerId) return;
        for (int i = m_Ownerships.Count() - 1; i >= 0; i--)
        {
            TM_TerritoryOwnership to = m_Ownerships.Get(i);
            if (to && to.ownerId == ownerId) m_Ownerships.RemoveOrdered(i);
        }
    }
    
    bool TM_CanBuildOrDismantle(PlayerBase player)
    {
        if (!player) return false;
        float searchR = DP_TerritoryConfig.Get().GetMaxPossibleRadius();
        Object flagObj = TM_GetNearestFlag(player.GetPosition(), searchR); 
        if (!flagObj) return true; 
        
        TerritoryFlag flag = TerritoryFlag.Cast(flagObj);
        if (flag)
        {
            string uid = player.GetIdentity().GetId();
            if (flag.HasPermissions(uid)) return true;
            return false; 
        }
        return true;
    }
}
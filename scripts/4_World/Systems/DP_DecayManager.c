class DP_DecayManager
{
    const float DECAY_INTERVAL = 3600.0; // 1 Hour
    const float BASE_DAMAGE = 50.0;
    const float MAX_SEARCH_RADIUS = 20000.0; // Maximum search radius for finding flags
    
    float m_Timer;

    void DP_DecayManager() 
    { 
        m_Timer = 0; 
    }

    void OnUpdate(float timeslice)
    {
        m_Timer += timeslice;
        if (m_Timer >= DECAY_INTERVAL) 
        { 
            ProcessDecay(); 
            m_Timer = 0; 
        }
    }

    void ProcessDecay()
    {
        Print("[DP_Decay] Starting decay cycle...");
        
        // Get all TerritoryFlag objects using manager instead of world search
        DP_TerritoryManager tm = DP_TerritoryManager.TM_GetInstance();
        if (!tm || !tm.m_Ownerships)
        {
            Print("[DP_Decay] Territory manager not initialized");
            return;
        }
        
        int flagsProcessed = 0;
        for (int i = 0; i < tm.m_Ownerships.Count(); i++)
        {
            TM_TerritoryOwnership ownership = tm.m_Ownerships.Get(i);
            if (!ownership || !ownership.flagObj) continue;
            
            TerritoryFlag flag = TerritoryFlag.Cast(ownership.flagObj);
            if (!flag) continue;

            int perkLevel = flag.GetPreservationPerkLevel(); 
            float damageMult = 1.0;
            
            // Apply perk-based damage reduction
            if (perkLevel == 1) damageMult = 0.8;
            else if (perkLevel == 2) damageMult = 0.6;
            else if (perkLevel >= 3) damageMult = 0.5;
            
            float finalDamage = BASE_DAMAGE * damageMult;
            float radius = flag.GetCurrentRadius();
            
            array<Object> baseObjects = new array<Object>;
            array<CargoBase> proxy = new array<CargoBase>;
            GetGame().GetObjectsAtPosition(flag.GetPosition(), radius, baseObjects, proxy);
            
            int objectsDecayed = 0;
            foreach(Object baseObj : baseObjects)
            {
                // Only decay fences and watchtowers
                if (baseObj.IsInherited(Fence) || baseObj.IsInherited(Watchtower))
                {
                    EntityAI building = EntityAI.Cast(baseObj);
                    if (building && !building.IsRuined()) 
                    {
                        building.AddHealth("", "", -finalDamage);
                        objectsDecayed++;
                    }
                }
            }
            
            flagsProcessed++;
            Print(string.Format("[DP_Decay] Flag %1: Processed %2 objects with %.1f damage (perk level %3)", 
                flagsProcessed, objectsDecayed, finalDamage, perkLevel));
        }
        
        Print(string.Format("[DP_Decay] Decay cycle completed. Processed %1 territories", flagsProcessed));
    }
}
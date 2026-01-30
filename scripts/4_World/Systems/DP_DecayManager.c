class DP_DecayManager
{
    const float DECAY_INTERVAL = 3600.0; // 1 Час
    const float BASE_DAMAGE = 50.0;
    
    float m_Timer;

    void DP_DecayManager() { m_Timer = 0; }

    void OnUpdate(float timeslice)
    {
        m_Timer += timeslice;
        if (m_Timer >= DECAY_INTERVAL) { ProcessDecay(); m_Timer = 0; }
    }

    void ProcessDecay()
    {
        Print("[DP_Decay] Start decay cycle...");
        array<Object> flags = new array<Object>;
        GetGame().GetObjectsAtPosition("0 0 0", 20000, flags, null);
        
        foreach(Object obj : flags)
        {
            TerritoryFlag flag = TerritoryFlag.Cast(obj);
            if (!flag) continue;

            int perkLevel = flag.GetPreservationPerkLevel(); 
            float damageMult = 1.0;
            if (perkLevel == 1) damageMult = 0.8;
            if (perkLevel == 2) damageMult = 0.6;
            if (perkLevel >= 3) damageMult = 0.5;
            
            float finalDamage = BASE_DAMAGE * damageMult;
            float radius = flag.GetCurrentRadius();
            
            array<Object> baseObjects = new array<Object>;
            array<CargoBase> proxy = new array<CargoBase>;
            GetGame().GetObjectsAtPosition(flag.GetPosition(), radius, baseObjects, proxy);
            
            foreach(Object baseObj : baseObjects)
            {
                if (baseObj.IsInherited(Fence) || baseObj.IsInherited(Watchtower))
                {
                    EntityAI building = EntityAI.Cast(baseObj);
                    if (building) building.AddHealth("", "", -finalDamage);
                }
            }
        }
        Print("[DP_Decay] End decay cycle.");
    }
}
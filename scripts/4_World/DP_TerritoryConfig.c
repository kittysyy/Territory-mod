class DP_CostItem
{
    string ClassName;
    string DisplayName;
    int Count;
    void DP_CostItem(string name, string disp, int c) { ClassName = name; DisplayName = disp; Count = c; }
}

class DP_LevelDefinition
{
    int Level;
    float Radius;
    int MaxStructures;
    int MaxTents;
    int MaxFurniture;
    ref array<ref DP_CostItem> UpgradeCost;

    void DP_LevelDefinition(int l, float r, int ms, int mt, int mf)
    {
        Level = l; Radius = r; MaxStructures = ms; MaxTents = mt; MaxFurniture = mf;
        UpgradeCost = new array<ref DP_CostItem>;
    }
}

class DP_TerritoryConfig
{
    ref array<string> StructureItems; 
    ref array<string> TentItems;      
    ref array<string> FurnitureItems; 
    ref array<ref DP_LevelDefinition> Levels;

    [NonSerialized()]
    static const string CONFIG_ROOT = "$profile:DarkProjectConfig";
    [NonSerialized()]
    static const string CONFIG_DIR  = "$profile:DarkProjectConfig/TerritoryMod";
    [NonSerialized()]
    static const string CONFIG_PATH = "$profile:DarkProjectConfig/TerritoryMod/DP_Territory_Config.json";
    
    [NonSerialized()]
    static ref DP_TerritoryConfig s_Instance;

    void DP_TerritoryConfig()
    {
        StructureItems = new array<string>;
        TentItems = new array<string>;
        FurnitureItems = new array<string>;
        Levels = new array<ref DP_LevelDefinition>;
    }
    
    static DP_TerritoryConfig Get()
    {
        if (!s_Instance) { s_Instance = new DP_TerritoryConfig(); s_Instance.Load(); }
        return s_Instance;
    }

    void Load()
    {
        if (!FileExist(CONFIG_ROOT)) MakeDirectory(CONFIG_ROOT);
        if (!FileExist(CONFIG_DIR)) MakeDirectory(CONFIG_DIR);
        if (FileExist(CONFIG_PATH)) { JsonFileLoader<DP_TerritoryConfig>.JsonLoadFile(CONFIG_PATH, this); }
        else { CreateDefault(); Save(); }
    }

    void Save()
    {
        if (!FileExist(CONFIG_ROOT)) MakeDirectory(CONFIG_ROOT);
        if (!FileExist(CONFIG_DIR)) MakeDirectory(CONFIG_DIR);
        JsonFileLoader<DP_TerritoryConfig>.JsonSaveFile(CONFIG_PATH, this);
    }

    void CreateDefault()
    {
        // Structure items (walls, watchtowers, etc.)
        StructureItems.Insert("Fence"); 
        StructureItems.Insert("Watchtower"); 
        StructureItems.Insert("FenceKit"); 
        StructureItems.Insert("WatchtowerKit");
        
        // Tent items
        TentItems.Insert("MediumTent"); 
        TentItems.Insert("LargeTent"); 
        TentItems.Insert("CarTent"); 
        TentItems.Insert("PartyTent");
        
        // Furniture items (storage containers)
        FurnitureItems.Insert("Barrel_ColorBase"); 
        FurnitureItems.Insert("SeaChest"); 
        FurnitureItems.Insert("WoodenCrate"); 
        FurnitureItems.Insert("BarrelHoles_ColorBase");
        
        // Level 1: Starter base
        DP_LevelDefinition l1 = new DP_LevelDefinition(1, 50.0, 10, 1, 5); 
        Levels.Insert(l1);

        // Level 2: Small base
        DP_LevelDefinition l2 = new DP_LevelDefinition(2, 100.0, 20, 2, 10);
        l2.UpgradeCost.Insert(new DP_CostItem("Nail", "Гвозди (шт)", 50)); 
        l2.UpgradeCost.Insert(new DP_CostItem("WoodenPlank", "Доски", 20)); 
        Levels.Insert(l2);
        
        // Level 3: Large base
        DP_LevelDefinition l3 = new DP_LevelDefinition(3, 150.0, 40, 5, 20);
        l3.UpgradeCost.Insert(new DP_CostItem("Nail", "Гвозди (шт)", 99)); 
        l3.UpgradeCost.Insert(new DP_CostItem("MetalPlate", "Листы железа", 10)); 
        l3.UpgradeCost.Insert(new DP_CostItem("WoodenLog", "Бревна", 5));
        Levels.Insert(l3);
    }
    
    DP_LevelDefinition GetLevelConfig(int level)
    {
        // Validate level parameter
        if (level < 1)
        {
            Print(string.Format("[DP_Territory WARNING] Invalid level requested: %1, returning level 1", level));
            level = 1;
        }
        
        // Search for exact level match
        for (int i = 0; i < Levels.Count(); i++)
        {
            DP_LevelDefinition levelDef = Levels.Get(i);
            if (levelDef && levelDef.Level == level) 
            {
                return levelDef;
            }
        }
        
        // Fallback: return first level if available
        if (Levels.Count() > 0) 
        {
            DP_LevelDefinition firstLevel = Levels.Get(0);
            if (firstLevel)
            {
                Print(string.Format("[DP_Territory WARNING] Level %1 not found, returning level 1", level));
                return firstLevel;
            }
        }
        
        // Last resort: return null
        Print("[DP_Territory ERROR] No levels defined in config!");
        return null;
    }
    
    float GetMaxPossibleRadius()
    {
        if (!Levels || Levels.Count() == 0)
        {
            Print("[DP_Territory WARNING] No levels defined, using default radius");
            return DP_TerritoryConstants.DEFAULT_RADIUS * DP_TerritoryConstants.DEFAULT_MAX_RADIUS_MULTIPLIER;
        }
        
        float maxR = 0;
        foreach(DP_LevelDefinition l : Levels) 
        { 
            if (l && l.Radius > maxR) 
            {
                maxR = l.Radius; 
            }
        }
        
        // If no valid radius found in config, use default
        if (maxR == 0) 
        {
            Print("[DP_Territory WARNING] No valid radius in config, using default");
            return DP_TerritoryConstants.DEFAULT_RADIUS * DP_TerritoryConstants.DEFAULT_MAX_RADIUS_MULTIPLIER;
        }
        
        return maxR;
    }
    
    int GetItemCategory(string classname)
    {
        if (IsTypeInList(classname, StructureItems)) return 1;
        if (IsTypeInList(classname, TentItems)) return 2;
        if (IsTypeInList(classname, FurnitureItems)) return 3;
        return 0;
    }
    
    bool IsTypeInList(string cls, array<string> list)
    {
        if (!list || !cls || cls == "") return false;
        
        foreach(string item : list) 
        { 
            if (GetGame().IsKindOf(cls, item)) 
            {
                return true; 
            }
        }
        
        return false;
    }
}
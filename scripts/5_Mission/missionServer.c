modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        
        Print("[DP_Territory] ------------------------------------------------");
        Print("[DP_Territory] ЗАГРУЗКА КОНФИГУРАЦИИ ТЕРРИТОРИИ...");
        DP_TerritoryConfig.Get(); 
        Print("[DP_Territory] Конфиг загружен успешно.");
        Print("[DP_Territory] ------------------------------------------------");
    }
}
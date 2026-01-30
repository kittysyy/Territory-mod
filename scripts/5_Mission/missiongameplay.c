modded class MissionGameplay
{
    override UIScriptedMenu CreateScriptedMenu(int id)
    {
        UIScriptedMenu menu = super.CreateScriptedMenu(id);
        if (menu) return menu;
        
        if (id == 777778)
        {
            menu = new DP_ManagementMenu;
            return menu;
        }

        return null;
    }

    override void OnKeyPress(int key)
    {
        super.OnKeyPress(key);

        if (key == KeyCode.KC_ESCAPE)
        {
            UIScriptedMenu menu = GetGame().GetUIManager().GetMenu();
            if (menu)
            {
                if (menu.ClassName() == "DP_ManagementMenu")
                {
                    menu.Close();
                }
            }
        }
    }
}
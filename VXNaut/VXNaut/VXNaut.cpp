#include "PluginSDK.h"
#include <map>

PluginSetup("VX.Naut by Veeox");

IMenu* MainMenu;

IMenu* ComboSettings;
IMenu* QSettings;
IMenu* WSettings;
IMenu* ESettings;
IMenu* RSettings;

IMenuOption* ComboQ;
IMenuOption* ComboW;
IMenuOption* ComboE;
IMenuOption* ComboR;

IMenu* HarassSettings;
IMenuOption* HarassQ;
IMenuOption* HarassW;
IMenuOption* HarassE;
IMenuOption* HarassPercent;

IMenu* MiscMenu;
IMenu* AutoQSettings;
std::map<int, IMenuOption*> AutoQ;
IMenuOption* AutoW;
IMenuOption* AutoInt;

IMenu* UltSettings;
std::map<int, IMenuOption*> UseUltOn;

IMenu* DrawingMenu;
IMenuOption* DrawQRange;
IMenuOption* DrawWRange;
IMenuOption* DrawERange;
IMenuOption* DrawRRange;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

ISpell* Ignite;

IUnit* myHero;



void InitializeMenu()
{
	MainMenu = GPluginSDK->AddMenu("VX.Naut");

	ComboSettings = MainMenu->AddMenu("Combo Settings");
	{
		ComboQ = ComboSettings->CheckBox("Use Q in combo", true);
		ComboW = ComboSettings->CheckBox("Use W in combo", true);
		ComboE = ComboSettings->CheckBox("Use E in combo", true);
		ComboR = ComboSettings->CheckBox("Use R in combo", true);
	}

	HarassSettings = MainMenu->AddMenu("Harass Settings");
	{
		HarassQ = HarassSettings->CheckBox("Use Q Harass", false);
		HarassE = HarassSettings->CheckBox("Use E Harass", false);
		HarassPercent = HarassSettings->AddInteger("Min mana % harass", 0, 100, 70);
	}

	UltSettings = MainMenu->AddMenu("Ultimate Settings");
	{
	for (auto Enemies : GEntityList->GetAllHeros(false, true))
	{
	if (Enemies != nullptr)
	{
		std::string name = "Ult: " + std::string(Enemies->ChampionName());
		UseUltOn[Enemies->GetNetworkId()] = UltSettings->CheckBox(name.c_str(), true);
	}
	}
	}

	MiscMenu = MainMenu->AddMenu("Misc Settings");
	{
		AutoQSettings = MiscMenu->AddMenu("Auto Q Settings");
		for (auto Enemies : GEntityList->GetAllHeros(false, true))
		{
			if (Enemies != nullptr)
			{
				std::string name = "Auto Q: " + std::string(Enemies->ChampionName());
				AutoQ[Enemies->GetNetworkId()] = AutoQSettings->CheckBox(name.c_str(), false);
			}
		}
		AutoW = MiscMenu->CheckBox("Auto W If Enemy In Range", false);
		AutoInt = MiscMenu->CheckBox("Auto Q To Interrupt", false);
	}
}

void LoadSpells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall, kCollidesWithMinions, kCollidesWithWalls));
	Q->SetSkillshot(0.25f, 250, 2000, 60);

	W = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, true, kCollidesWithNothing);
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, true, kCollidesWithYasuoWall);
}

int GetEnemiesInRange(float range)
{
	auto enemies = GEntityList->GetAllHeros(false, true);
	auto enemiesInRange = 0;

	for (auto enemy : enemies)
	{
		if (enemy != nullptr && enemy->GetTeam() != GEntityList->Player()->GetTeam())
		{
			auto flDistance = (enemy->GetPosition() - GEntityList->Player()->GetPosition()).Length();
			if (flDistance < range)
			{
				enemiesInRange++;
			}
		}
	}
	return enemiesInRange;
}

void Combo()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (ComboQ->Enabled() && Q->IsReady() && Q->Range())
		{
			if (Enemy != nullptr)
			{
				Q->CastOnTarget(Enemy, kHitChanceHigh);
			}
		}

		if (ComboW->Enabled() && W->IsReady())
		{
			if (Enemy != nullptr && (Enemy->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 175)
			{
				W->CastOnPlayer();
			}
		}

		if (ComboE->Enabled() && E->IsReady() && Q->Range())
		{
			if (Enemy != nullptr && (Enemy->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 600)
			{
				E->CastOnPlayer();
			}
		}

		if (ComboR->Enabled() && R->IsReady() && R->Range())
		{
			if (UseUltOn[Enemy->GetNetworkId()]->Enabled() && Enemy != nullptr && !Enemy->IsDead() && !Enemy->IsInvulnerable())
			{
				R->CastOnTarget(Enemy);
			}
		}
	}
}

void Harass()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (HarassQ->Enabled() && Q->IsReady() && Q->Range())
		{
			if (Enemy != nullptr)
			{
				Q->CastOnTarget(Enemy, kHitChanceHigh);
			}
		}

		if (HarassE->Enabled() && E->IsReady() && Q->Range())
		{
			if (Enemy != nullptr && (Enemy->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 600)
			{
				E->CastOnPlayer();
			}
		}
	}
}

void AutoWInRange()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (AutoW->Enabled() && W->IsReady() && (Enemy->GetPosition() - GEntityList->Player()->GetPosition()).Length() < 120)
		{
			W->CastOnPlayer();
		}
	}
}

void AutoHook()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (AutoQ[Enemy->GetNetworkId()]->Enabled())
		{
			auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
			if (target != nullptr)
			{
				if (Q->IsReady())
				{
					Q->CastOnTarget(target);
				}
			}
		}
	}
}

PLUGIN_EVENT(void) OnInterruptible(InterruptibleSpell const& Args)

{
	if (AutoInt->Enabled() && (Args.Target->GetPosition() - GEntityList->Player()->GetPosition()).Length() < Q->Range() && Q->IsReady() && Args.Target->IsValidTarget())
	{
		Q->CastOnTarget(Args.Target);
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	if (GEntityList->Player()->IsDead())
		return;

	AutoWInRange();

	switch (GOrbwalking->GetOrbwalkingMode())
	{
	case kModeCombo:
		Combo();
		break;
	case kModeMixed:
		Harass();
		break;
	default:;
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	InitializeMenu();
	LoadSpells();

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
}
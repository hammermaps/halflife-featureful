#include <gtest/gtest.h>
#include "ent_templates.h"
#include "blood_types.h"
#include "classify.h"
#include "grapple_target.h"

#include <algorithm>

const char entities[] = R"(
{
	"female_civ": {
		"own_visual": {
			"model": "models/us_gal.mdl"
		},
		"soundscripts": {
			"Civilian.Pain": {
				"waves": ["colette/colette_pain0.wav", "colette/colette_pain1.wav"]
			},
			"Civilian.Die": "Female.Die"
		},
		"speech_prefix": "FEM"
	},
	"vort_friendly": {
		"classify": "Player Ally",
		"field_of_view": "full",
		"size_for_grapple": "no",
		"visuals": {
			"Vortigaunt.ZapBeamColor": {
				"color": [255, 96, 180]
			},
			"Vortigaunt.ArmBeamColor": "Friendly.BeamColor"
		}
	},
	"monster_gonome": {
		"precached_sounds": ["gonome/gonome_step1.wav", "gonome/gonome_step2.wav"],
		"blood": "red",
		"health": 225,
		"field_of_view": 0.3
	},
	"monster_gargantua": {
		"gib_visual": {
			"model": "models/garg_gibs.mdl"
		},
		"blood": "no",
		"size": {
			"mins": [-40,-40,0],
			"maxs": [40,40,214]
		}
	},
	"small_bullsquid": {
		"size": "small",
		"size_for_grapple": "small"
	}
}
)";

TEST(EntityTemplates, Parse)
{
	SoundScriptSystem ss;
	VisualSystem vs;
	EntTemplateSystem es;
	es.SetSoundScriptSystem(&ss);
	es.SetVisualSystem(&vs);

	ASSERT_TRUE(es.ReadFromContents(entities, ""));

	{
		const EntTemplate* femaleCiv = es.GetTemplate("female_civ");
		ASSERT_TRUE(femaleCiv != nullptr);

		EXPECT_STREQ(femaleCiv->OwnVisualName(), "female_civ##own_visual");
		const Visual* ownVisual = vs.GetVisual("female_civ##own_visual");
		ASSERT_TRUE(ownVisual != nullptr);
		EXPECT_STREQ(ownVisual->model, "models/us_gal.mdl");

		EXPECT_STREQ(femaleCiv->GetSoundScriptNameOverride("Civilian.Pain"), "female_civ#Civilian.Pain");
		const SoundScript* painSoundScript = ss.GetSoundScript("female_civ#Civilian.Pain");
		ASSERT_TRUE(painSoundScript != nullptr);
		EXPECT_EQ(painSoundScript->waves.size(), 2);
		EXPECT_STREQ(painSoundScript->waves[0], "colette/colette_pain0.wav");

		EXPECT_STREQ(femaleCiv->GetSoundScriptNameOverride("Civilian.Die"), "Female.Die");
		EXPECT_TRUE(femaleCiv->GetSoundScriptNameOverride("Nonexistent") == nullptr);

		EXPECT_EQ(femaleCiv->PrecachedSoundsBegin(), femaleCiv->PrecachedSoundsEnd());
		EXPECT_EQ(femaleCiv->PrecachedSoundScriptsBegin(), femaleCiv->PrecachedSoundScriptsEnd());

		EXPECT_FALSE(femaleCiv->IsClassifyDefined());
		EXPECT_FALSE(femaleCiv->IsBloodDefined());ASSERT_TRUE(femaleCiv != nullptr);
		EXPECT_FALSE(femaleCiv->IsHealthDefined());
		EXPECT_FALSE(femaleCiv->IsFielfOfViewDefined());
		EXPECT_FALSE(femaleCiv->IsSizeDefined());
		EXPECT_FALSE(femaleCiv->IsSizeForGrappleDefined());

		EXPECT_STREQ(femaleCiv->SpeechPrefix(), "FEM");
	}

	{
		const EntTemplate* vortFriendly = es.GetTemplate("vort_friendly");
		ASSERT_TRUE(vortFriendly != nullptr);

		EXPECT_TRUE(vortFriendly->OwnVisualName() == nullptr);
		EXPECT_TRUE(vortFriendly->GibVisualName() == nullptr);

		EXPECT_TRUE(vortFriendly->IsClassifyDefined());
		EXPECT_EQ(vortFriendly->Classify(), CLASS_PLAYER_ALLY);

		EXPECT_TRUE(vortFriendly->IsFielfOfViewDefined());
		EXPECT_EQ(vortFriendly->FieldOfView(), -1.0f);

		EXPECT_TRUE(vortFriendly->IsSizeForGrappleDefined());
		EXPECT_EQ(vortFriendly->SizeForGrapple(), GRAPPLE_NOT_A_TARGET);

		EXPECT_STREQ(vortFriendly->GetVisualNameOverride("Vortigaunt.ZapBeamColor"), "vort_friendly#Vortigaunt.ZapBeamColor");
		EXPECT_STREQ(vortFriendly->GetVisualNameOverride("Vortigaunt.ArmBeamColor"), "Friendly.BeamColor");
	}

	{
		const EntTemplate* gonome = es.GetTemplate("monster_gonome");
		ASSERT_TRUE(gonome != nullptr);

		auto it = gonome->PrecachedSoundsBegin();
		EXPECT_EQ(*it, "gonome/gonome_step1.wav");
		++it;
		EXPECT_EQ(*it, "gonome/gonome_step2.wav");
		++it;
		EXPECT_EQ(it, gonome->PrecachedSoundsEnd());

		EXPECT_TRUE(gonome->IsBloodDefined());
		EXPECT_EQ(gonome->BloodColor(), BLOOD_COLOR_RED);

		EXPECT_TRUE(gonome->IsHealthDefined());
		EXPECT_EQ(gonome->Health(), 225.0f);

		EXPECT_TRUE(gonome->IsFielfOfViewDefined());
		EXPECT_EQ(gonome->FieldOfView(), 0.3f);
	}

	{
		const EntTemplate* garg = es.GetTemplate("monster_gargantua");
		ASSERT_TRUE(garg != nullptr);

		EXPECT_STREQ(garg->GibVisualName(), "monster_gargantua##gib_visual");
		const Visual* gibVisual = vs.GetVisual("monster_gargantua##gib_visual");
		ASSERT_TRUE(gibVisual != nullptr);
		EXPECT_STREQ(gibVisual->model, "models/garg_gibs.mdl");

		EXPECT_TRUE(garg->IsBloodDefined());
		EXPECT_EQ(garg->BloodColor(), DONT_BLEED);

		EXPECT_TRUE(garg->IsSizeDefined());
		EXPECT_EQ(garg->MinSize(), Vector(-40.0f, -40.0f, 0.0f));
		EXPECT_EQ(garg->MaxSize(), Vector(40.0f, 40.0f, 214.0f));
	}

	{
		const EntTemplate* smallSquid = es.GetTemplate("small_bullsquid");
		ASSERT_TRUE(smallSquid != nullptr);

		EXPECT_TRUE(smallSquid->IsSizeDefined());
		EXPECT_EQ(smallSquid->MinSize(), Vector(-16.0f, -16.0f, 0.0f));
		EXPECT_EQ(smallSquid->MaxSize(), Vector(16.0f, 16.0f, 36.0f));

		EXPECT_TRUE(smallSquid->IsSizeForGrappleDefined());
		EXPECT_EQ(smallSquid->SizeForGrapple(), GRAPPLE_SMALL);
	}

	{
		const EntTemplate* nonExistent = es.GetTemplate("nonexistent");
		EXPECT_TRUE(nonExistent == nullptr);
	}
}

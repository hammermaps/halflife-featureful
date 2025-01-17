#include "ent_templates.h"
#include "icase_compare.h"

#include "blood_types.h"
#include "classify.h"
#include "grapple_target.h"
#include "hull_sizes.h"

#include <set>
#include <utility>

#include "json_utils.h"
#include "logger.h"

using namespace rapidjson;

const char* entTemplatesSchema = R"(
{
	"definitions": {
		"template": {
			"type": "object",
			"properties": {
				"own_visual": {
					"$ref": "definitions.json#/visual"
				},
				"gib_visual": {
					"$ref": "definitions.json#/visual"
				},
				"size": {
					"type": ["object", "string"],
					"properties": {
						"mins": "definitions.json#/vector",
						"maxs": "definitions.json#/vector"
					},
					"additionalProperties": false,
					"required": ["mins", "maxs"]
				},
				"classify": {
					"type": "string"
				},
				"blood": {
					"type": "string"
				},
				"field_of_view": {
					"type": ["number", "string"],
					"minimum": -1.0,
					"exclusiveMaximum": 1.0
				},
				"health": {
					"type": "number",
					"exclusiveMinimum": 0.0
				},
				"soundscripts": {
					"type": "object",
					"additionalProperties": {
						"$ref": "definitions.json#/soundscript"
					}
				},
				"visuals": {
					"type": "object",
					"additionalProperties": {
						"$ref": "definitions.json#/visual"
					}
				},
				"sound_replacement": {
					"type": "object",
					"additionalProperties": {
						"type": "string"
					}
				},
				"precached_sounds": {
					"type": "array",
					"items": {
						"type": "string"
					}
				},
				"precached_soundscripts": {
					"type": "array",
					"items": {
						"type": "string"
					}
				},
				"autoprecache_sounds": {
					"type": "boolean"
				},
				"autoprecache_soundscripts": {
					"type": "boolean"
				},
				"size_for_grapple": {
					"type": "string"
				},
				"speech_prefix": {
					"type": "string",
					"minLength": 1
				}
			},
			"additionalProperties": false
		}
	},
	"type": "object",
	"additionalProperties": {
		"$ref": "#/definitions/template"
	}
}
)";

static std::string g_tempString;

const char* EntTemplate::OwnVisualName() const
{
	return _ownVisual.empty() ? nullptr : _ownVisual.c_str();
}

const char* EntTemplate::GibVisualName() const
{
	return _gibVisual.empty() ? nullptr : _gibVisual.c_str();
}

const char* EntTemplate::GetSoundScriptNameOverride(const char *name) const
{
	g_tempString = name;
	auto it = _soundScripts.find(g_tempString);
	if (it != _soundScripts.end())
		return it->second.c_str();
	return nullptr;
}

const char* EntTemplate::GetVisualNameOverride(const char *name) const
{
	g_tempString = name;
	auto it = _visuals.find(g_tempString);
	if (it != _visuals.end())
		return it->second.c_str();
	return nullptr;
}

void EntTemplate::SetSoundScriptReplacement(const char *soundScript, const std::string& replacement)
{
	_soundScripts[soundScript] = replacement;
}

void EntTemplate::SetVisualReplacement(const char *visual, const std::string& replacement)
{
	_visuals[visual] = replacement;
}

const char* EntTemplate::GetSoundReplacement(const char *originalSample) const
{
	if (_soundReplacements.empty())
		return nullptr;
	auto it = _soundReplacements.find(originalSample);
	if (it == _soundReplacements.end())
		return nullptr;
	return it->second.c_str();
}

void EntTemplate::SetSoundReplacement(const char *originalSample, const char *replacementSample)
{
	_soundReplacements[originalSample] = replacementSample;
}

void EntTemplate::SetPrecachedSounds(std::vector<std::string> &&sounds)
{
	_precachedSounds = sounds;
}

void EntTemplate::SetPrecachedSoundScripts(std::vector<std::string> &&soundScripts)
{
	_precachedSoundScripts = soundScripts;
}

const char* EntTemplate::SpeechPrefix() const
{
	return _speechPrefix.empty() ? nullptr : _speechPrefix.c_str();
}

static std::string GenerateResourceName(const std::string& templateName, const char* resourceName)
{
	return templateName + '#' + resourceName;
}

const char* EntTemplateSystem::Schema() const
{
	return entTemplatesSchema;
}

bool EntTemplateSystem::ReadFromDocument(rapidjson::Document& document, const char* fileName)
{
	for (auto templateIt = document.MemberBegin(); templateIt != document.MemberEnd(); ++templateIt)
	{
		const std::string templateName = templateIt->name.GetString();

		EntTemplate entTemplate;

		Value& value = templateIt->value;

		{
			auto it = value.FindMember("own_visual");
			if (it != value.MemberEnd())
			{
				if (it->value.IsString())
				{
					entTemplate.SetOwnVisualName(it->value.GetString());
				}
				else if (it->value.IsObject())
				{
					std::string ownVisualName = templateName + "##own_visual";
					entTemplate.SetOwnVisualName(ownVisualName);
					_visualSystem->AddVisualFromJsonValue(ownVisualName.c_str(), it->value);
				}
			}
		}

		{
			auto it = value.FindMember("gib_visual");
			if (it != value.MemberEnd())
			{
				if (it->value.IsString())
				{
					entTemplate.SetGibVisualName(it->value.GetString());
				}
				else if (it->value.IsObject())
				{
					std::string gibVisualName = templateName + "##gib_visual";
					entTemplate.SetGibVisualName(gibVisualName);
					_visualSystem->AddVisualFromJsonValue(gibVisualName.c_str(), it->value);
				}
			}
		}

		{
			auto it = value.FindMember("soundscripts");
			if (it != value.MemberEnd())
			{
				Value& soundScriptsObj = it->value;
				for (auto scriptIt = soundScriptsObj.MemberBegin(); scriptIt != soundScriptsObj.MemberEnd(); ++scriptIt)
				{
					const char* soundScriptName = scriptIt->name.GetString();
					if (scriptIt->value.IsString())
					{
						const char* replacement = scriptIt->value.GetString();
						entTemplate.SetSoundScriptReplacement(soundScriptName, replacement);
					}
					else if (scriptIt->value.IsObject())
					{
						std::string replacement = GenerateResourceName(templateName, soundScriptName);
						entTemplate.SetSoundScriptReplacement(soundScriptName, replacement);
						_soundScriptSystem->AddSoundScriptFromJsonValue(replacement.c_str(), scriptIt->value);
					}
				}
			}
		}

		{
			auto it = value.FindMember("visuals");
			if (it != value.MemberEnd())
			{
				Value& visualsObj = it->value;
				for (auto visualIt = visualsObj.MemberBegin(); visualIt != visualsObj.MemberEnd(); ++visualIt)
				{
					const char* visualName = visualIt->name.GetString();
					if (visualIt->value.IsString())
					{
						const char* replacement = visualIt->value.GetString();
						entTemplate.SetVisualReplacement(visualName, replacement);
					}
					else if (visualIt->value.IsObject())
					{
						std::string replacement = GenerateResourceName(templateName, visualName);
						entTemplate.SetVisualReplacement(visualName, replacement);
						_visualSystem->AddVisualFromJsonValue(replacement.c_str(), visualIt->value);
					}
				}
			}
		}

		{
			auto it = value.FindMember("sound_replacement");
			if (it != value.MemberEnd())
			{
				Value& soundReplacement = it->value;
				for (auto sIt = soundReplacement.MemberBegin(); sIt != soundReplacement.MemberEnd(); ++sIt)
				{
					const char* originalSample = sIt->name.GetString();
					const char* replacementSample = sIt->value.GetString();
					entTemplate.SetSoundReplacement(originalSample, replacementSample);
				}
			}
		}

		{
			auto it = value.FindMember("precached_sounds");
			if (it != value.MemberEnd())
			{
				Value::Array arr = it->value.GetArray();
				std::vector<std::string> sounds;
				for (size_t i=0; i<arr.Size(); ++i)
				{
					sounds.push_back(arr[i].GetString());
				}
				entTemplate.SetPrecachedSounds(std::move(sounds));
			}
		}

		{
			auto it = value.FindMember("precached_soundscripts");
			if (it != value.MemberEnd())
			{
				Value::Array arr = it->value.GetArray();
				std::vector<std::string> soundsScripts;
				for (size_t i=0; i<arr.Size(); ++i)
				{
					soundsScripts.push_back(arr[i].GetString());
				}
				entTemplate.SetPrecachedSoundScripts(std::move(soundsScripts));
			}
		}

		{
			bool b;
			if (UpdatePropertyFromJson(b, value, "autoprecache_sounds"))
			{
				entTemplate.SetAutoPrecacheSounds(b);
			}
		}

		{
			bool b;
			if (UpdatePropertyFromJson(b, value, "autoprecache_soundscripts"))
			{
				entTemplate.SetAutoPrecacheSoundScripts(b);
			}
		}

		{
			auto it = value.FindMember("classify");
			if (it != value.MemberEnd())
			{
				const char* classifyName = it->value.GetString();
				int classify = ClassifyFromName(classifyName);
				if (classify < 0)
				{
					LOG("Unknown classification '%s'\n", classifyName);
				}
				else
				{
					entTemplate.SetClassify(classify);
				}
			}
		}

		{
			auto it = value.FindMember("blood");
			if (it != value.MemberEnd())
			{
				const char* bloodType = it->value.GetString();
				if (stricmp(bloodType, "red") == 0)
				{
					entTemplate.SetBloodColor(BLOOD_COLOR_RED);
				}
				else if (stricmp(bloodType, "yellow") == 0)
				{
					entTemplate.SetBloodColor(BLOOD_COLOR_YELLOW);
				}
				else if (stricmp(bloodType, "no") == 0)
				{
					entTemplate.SetBloodColor(DONT_BLEED);
				}
				else
				{
					LOG_WARNING("Unknown blood type '%s'\n", bloodType);
				}
			}
		}

		{
			auto it = value.FindMember("health");
			if (it != value.MemberEnd())
			{
				entTemplate.SetHealth(it->value.GetFloat());
			}
		}

		{
			auto it = value.FindMember("field_of_view");
			if (it != value.MemberEnd())
			{
				if (it->value.IsNumber())
				{
					entTemplate.SetFieldOfView(it->value.GetFloat());
				}
				else if (it->value.IsString())
				{
					const char* fovType = it->value.GetString();
					const std::pair<const char*, float> fovPairs[] = {
						{"full", -1.0f},
						{"wide", -0.7f},
						{"average", 0.2f},
						{"tunnel", 0.5f},
						{"narrow", 0.7f},
					};

					bool foundFov = false;
					for (auto p: fovPairs)
					{
						if (stricmp(p.first, fovType) == 0)
						{
							foundFov = true;
							entTemplate.SetFieldOfView(p.second);
							break;
						}
					}

					if (!foundFov)
						LOG_WARNING("Unknown FOV type '%s'\n", fovType);
				}
			}
		}

		{
			auto it = value.FindMember("size");
			if (it != value.MemberEnd())
			{
				if (it->value.IsString())
				{
					const char* sizePreset = it->value.GetString();
					if (stricmp(sizePreset, "snark") == 0)
					{
						entTemplate.SetSize(Vector(-4.0f, -4.0f, 0.0f), Vector(4.0f, 4.0f, 8.0f));
					}
					else if (stricmp(sizePreset, "headcrab") == 0)
					{
						entTemplate.SetSize(Vector(-12.0f, -12.0f, 0.0f), Vector(12.0f, 12.0f, 24.0f));
					}
					else if (stricmp(sizePreset, "small") == 0)
					{
						entTemplate.SetSize(Vector(-16.0f, -16.0f, 0.0f), Vector( 16.0f, 16.0f, 36.0f ));
					}
					else if (stricmp(sizePreset, "human") == 0)
					{
						entTemplate.SetSize(VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
					}
					else if (stricmp(sizePreset, "large") == 0)
					{
						entTemplate.SetSize(Vector(-32.0f, -32.0f, 0.0f), Vector(32.0f, 32.0f, 64.0f));
					}
					else
					{
						LOG_WARNING("Unknown size preset '%s'\n", sizePreset);
					}
				}
				else if (it->value.IsObject())
				{
					Vector minVec, maxVec;
					if (UpdatePropertyFromJson(minVec, it->value, "mins") && UpdatePropertyFromJson(maxVec, it->value, "maxs"))
					{
						entTemplate.SetSize(minVec, maxVec);
					}
				}
			}
		}

		{
			auto it = value.FindMember("size_for_grapple");
			if (it != value.MemberEnd())
			{
				const char* targetType = it->value.GetString();
				const std::pair<const char*, int> sizeValues[] = {
					{"no", GRAPPLE_NOT_A_TARGET},
					{"small", GRAPPLE_SMALL},
					{"medium", GRAPPLE_MEDIUM},
					{"large", GRAPPLE_LARGE},
					{"fixed", GRAPPLE_FIXED},
				};

				bool found = false;
				for (auto p: sizeValues)
				{
					if (stricmp(p.first, targetType) == 0)
					{
						found = true;
						entTemplate.SetSizeForGrapple(p.second);
						break;
					}
				}

				if (!found)
					LOG_WARNING("Unknown grapple target type '%s'\n", targetType);
			}
		}

		{
			auto it = value.FindMember("speech_prefix");
			if (it != value.MemberEnd())
			{
				entTemplate.SetSpeechPrefix(it->value.GetString());
			}
		}

		_entTemplates[templateName] = entTemplate;
	}

	return true;
}

const EntTemplate* EntTemplateSystem::GetTemplate(const char *name)
{
	if (!name || *name == '\0')
		return nullptr;
	_temp = name;
	auto it = _entTemplates.find(_temp);
	if (it != _entTemplates.end())
		return &it->second;
	return nullptr;
}

void EntTemplateSystem::EnsureVisualReplacementForTemplate(const char* templateName, const char* visualName)
{
	if (!templateName || *templateName == '\0')
		return;
	_temp = templateName;
	auto it = _entTemplates.find(_temp);
	if (it != _entTemplates.end())
	{
		EntTemplate* entTemplate = &it->second;
		if (!entTemplate->GetVisualNameOverride(visualName))
		{
			std::string replacement = GenerateResourceName(it->first, visualName);
			entTemplate->SetVisualReplacement(visualName, replacement);
			_visualSystem->EnsureVisualExists(replacement);
		}
	}
}

void EntTemplateSystem::EnsureSoundScriptReplacementForTemplate(const char* templateName, const char* soundScriptName)
{
	if (!templateName || *templateName == '\0')
		return;
	_temp = templateName;
	auto it = _entTemplates.find(_temp);
	if (it != _entTemplates.end())
	{
		EntTemplate* entTemplate = &it->second;
		if (!entTemplate->GetSoundScriptNameOverride(soundScriptName))
		{
			std::string replacement = GenerateResourceName(it->first, soundScriptName);
			entTemplate->SetSoundScriptReplacement(soundScriptName, replacement);
			_soundScriptSystem->EnsureSoundScriptExists(replacement);
		}
	}
}

EntTemplateSystem g_EntTemplateSystem;

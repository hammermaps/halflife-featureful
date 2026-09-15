---
bookHidden: true
bookToC: false
entityCategory: object
---

# xen_tree

[TWHL](https://twhl.info/wiki/page/xen_tree)

{{% hint info %}}
This entity can play sound [animation events]({{< ref model-animation-events >}}). Use entity template with [autoprecache_sounds]({{< ref "entity-templates/#autoprecache_sounds" >}}) enabled to precache such sounds.
{{% /hint %}}

### Bugfixes

* Going through the level transition and back and then attacking or touching the xen tree won't crash the game anymore.

### New parameters

* `Custom model`

### New spawnflags

* ![](/images/svencoop.png) `Drop to Floor`.
* ![](/images/svencoop.png) `Non Solid`.
* `Go across changelevel` - allow xen plant travel across level transition.

### Soundscripts

* **XenTree.AttackHit** - hitting someone. Derived from **NPC.AttackHit**
* **XenTree.AttackMiss** - attack miss. Derived from **NPC.AttackMiss**

### Animation events

* `1` - find anyone in the impact zone and deal damage.

### Entity template examples

{{% tabs %}}

{{% tab "Custom damage" %}}

Custom damage properties can be set via entity template's [trace_hull_attacks]({{< ref "entity-templates/#trace_hull_attacks" >}}).

E.g. the following entity template lets the xen tree deal poison damage and make the hit target bleed.

```json
{
    "xen_tree": {
        "trace_hull_attacks": {
            "1": {
                "damage_info": {
                    "damage": 25,
                    "type": ["poison"],
                    "type_policy": "add"
                },
                "spawn_blood": true
            }
        }
    }
}
```
{{% /tab %}}

{{% /tabs %}}

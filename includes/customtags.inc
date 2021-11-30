#if defined __confogl_customtags_included
	#endinput
#endif
#define __confogl_customtags_included

// COPYRIGHT PSYCHONIC
// USED WITH PERMISSION

#define SV_TAG_SIZE 64

static stock EngineVersion
	engine_version = Engine_Unknown;

static stock bool
	are_tags_hooked = false,
	ignore_next_change = false;

static stock ConVar
	sv_tags = null;

static stock ArrayList
	custom_tags = null;

stock void AddCustomServerTag(const char[] tag, bool force = false)
{
	if (sv_tags == null && (sv_tags = FindConVar("sv_tags")) == null) {
		// game doesn't support sv_tags
		return;
	}

	if (!force
		&& (engine_version != Engine_Unknown || (engine_version = GetEngineVersion()) != Engine_Unknown)
		&& engine_version >= Engine_Left4Dead2
	) {
		return;
	}

	if (custom_tags == null) {
		custom_tags = new ArrayList(SV_TAG_SIZE);
		custom_tags.PushString(tag);
	} else if (custom_tags.FindString(tag) == -1) {
		custom_tags.PushString(tag);
	}

	char current_tags[SV_TAG_SIZE];
	sv_tags.GetString(current_tags, sizeof(current_tags));

	if (StrContains(current_tags, tag) > -1) {
		// already have tag
		return;
	}

	char new_tags[SV_TAG_SIZE];
	Format(new_tags, sizeof(new_tags), "%s%s%s", current_tags, (current_tags[0] != 0) ? "," : "", tag);

	int flags = sv_tags.Flags;
	sv_tags.Flags = flags & ~FCVAR_NOTIFY;

	ignore_next_change = true;
	sv_tags.SetString(new_tags);
	ignore_next_change = false;

	sv_tags.Flags = flags;

	if (!are_tags_hooked) {
		sv_tags.AddChangeHook(OnTagsChanged);
		are_tags_hooked = true;
	}
}

stock void RemoveCustomServerTag(const char[] tag)
{
	if (sv_tags == null && (sv_tags = FindConVar("sv_tags")) == null) {
		// game doesn't support sv_tags
		return;
	}

	// we wouldn't have to check this if people aren't removing before adding, but... you know...
	if (custom_tags != null) {
		int idx = custom_tags.FindString(tag);
		if (idx > -1) {
			custom_tags.Erase(idx);
		}
	}

	char current_tags[SV_TAG_SIZE];
	sv_tags.GetString(current_tags, sizeof(current_tags));

	if (StrContains(current_tags, tag) == -1) {
		// tag isn't on here, just bug out
		return;
	}

	ReplaceString(current_tags, sizeof(current_tags), tag, "");
	ReplaceString(current_tags, sizeof(current_tags), ",,", "");

	int flags = sv_tags.Flags;
	sv_tags.Flags = flags & ~FCVAR_NOTIFY;

	ignore_next_change = true;
	sv_tags.SetString(current_tags);
	ignore_next_change = false;

	sv_tags.Flags = flags;
}

public void OnTagsChanged(ConVar hConvar, const char[] sOldValue, const char[] sNewValue)
{
	if (ignore_next_change) {
		// we fired this callback, no need to reapply tags
		return;
	}

	// reapply each custom tag
	char tag[SV_TAG_SIZE];
	int iSize = custom_tags.Length;

	for (int i = 0; i < iSize; i++) {
		custom_tags.GetString(i, tag, sizeof(tag));
		AddCustomServerTag(tag);
	}
}

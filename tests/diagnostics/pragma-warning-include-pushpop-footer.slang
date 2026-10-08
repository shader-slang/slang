implementing pragma_pushpop_test;

// A push/pop that touches only an unrelated id (31000 is just some other diagnostic id, not
// 30856). It still used to disturb the root module's disable of 30856 because
// `WarningStateTracker::addPragmaPop` restores *every* known id on `pop`, not just the pushed
// one — so combined with the cross-file location collision, popping here re-enabled 30856 for the
// earlier body file. After the fix this push/pop must leave the root disable of 30856 intact.
#pragma warning(push)
#pragma warning(disable: 31000)
#pragma warning(pop)

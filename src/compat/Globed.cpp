#include <Geode/Geode.hpp>
#include <managers/LevelManager.hpp>

using namespace geode::prelude;
using namespace tulip::editor;


namespace globed {
    struct SetupLevelIdEvent : public geode::Event<SetupLevelIdEvent, bool(GJGameLevel*, int64_t*)> {
        using Event::Event;
    };
}

namespace tulip::editor {
	using LevelKey = uint64_t;

	class LevelKeyImpl {
	public:
		static LevelKey fromString(std::string const& str) {
			auto ret = LevelKey();
			for (auto c : str) {
				ret <<= 7;
				ret += c;
			}
			return ret;
		}

		static std::string toString(LevelKey key) {
			auto ret = std::string();
			while (key > 0) {
				auto c = static_cast<char>(key & 127);
				ret = c + ret;
				key >>= 7;
			}
			return ret;
		}
	};
}

$on_mod(Loaded) {
    globed::SetupLevelIdEvent().listen(+[](GJGameLevel* level, int64_t* id) {
        if (LevelManager::get()->hasJoinedLevelKey()) {
            *id = (int64_t)LevelKeyImpl::fromString(LevelManager::get()->getJoinedLevelKey());
            return ListenerResult::Stop;
        }
		return ListenerResult::Propagate;
	}).leak();
}
#ifndef SORBET_INSTRUCTIONS_H
#define SORBET_INSTRUCTIONS_H

#include "core/core.h"
#include <climits>
#include <memory>
#include <unordered_map>
#include <unordered_set>

namespace sorbet {
namespace cfg {

// TODO: convert it to implicitly numbered instead of explicitly bound
// implicitly numbered: result of every instruction can be uniquely referenced
// by its position in a linear array.

// When adding a new subtype, see if you need to add it to fillInBlockArguments
class Instruction {
public:
    virtual ~Instruction() = default;
    virtual std::string toString(core::Context ctx) = 0;
    Instruction() = default;
    bool isSynthetic = false;
};

template <class To> To *cast_instruction(Instruction *what) {
    static_assert(!std::is_pointer<To>::value, "To has to be a pointer");
    static_assert(std::is_assignable<Instruction *&, To *>::value,
                  "Ill Formed To, has to be a subclass of Instruction");
    return fast_cast<Instruction, To>(what);
}

template <class To> bool isa_instruction(Instruction *what) {
    return cast_instruction<To>(what) != nullptr;
}

class Ident final : public Instruction {
public:
    core::LocalVariable what;

    Ident(core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};

class Alias final : public Instruction {
public:
    core::SymbolRef what;

    Alias(core::SymbolRef what);

    virtual std::string toString(core::Context ctx);
};

class SolveConstraint final : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    SolveConstraint(std::shared_ptr<core::SendAndBlockLink> link) : link(link){};
    virtual std::string toString(core::Context ctx);
};

class Send final : public Instruction {
public:
    core::LocalVariable recv;
    core::NameRef fun;
    core::Loc receiverLoc;
    std::vector<core::LocalVariable> args;
    std::vector<core::Loc> argLocs;
    std::shared_ptr<core::SendAndBlockLink> link;

    Send(core::LocalVariable recv, core::NameRef fun, core::Loc receiverLoc, std::vector<core::LocalVariable> &args,
         std::vector<core::Loc> &argLocs, std::shared_ptr<core::SendAndBlockLink> link = nullptr);

    virtual std::string toString(core::Context ctx);
};

class Return final : public Instruction {
public:
    core::LocalVariable what;

    Return(core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};

class BlockReturn final : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    core::LocalVariable what;

    BlockReturn(std::shared_ptr<core::SendAndBlockLink> link, core::LocalVariable what);
    virtual std::string toString(core::Context ctx);
};

class Literal final : public Instruction {
public:
    std::shared_ptr<core::Type> value;

    Literal(std::shared_ptr<core::Type> value);
    virtual std::string toString(core::Context ctx);
};

class Unanalyzable : public Instruction {
public:
    Unanalyzable() {
        core::categoryCounterInc("cfg", "unanalyzable");
    };
    virtual std::string toString(core::Context ctx);
};

class NotSupported final : public Unanalyzable {
public:
    std::string why;

    NotSupported(std::string why) : why(why) {
        core::categoryCounterInc("cfg", "notsupported");
    };
    virtual std::string toString(core::Context ctx);
};

class Self final : public Instruction {
public:
    core::SymbolRef klass;

    Self(core::SymbolRef klass) : klass(klass) {
        core::categoryCounterInc("cfg", "self");
    };
    virtual std::string toString(core::Context ctx);
};

class LoadArg final : public Instruction {
public:
    core::LocalVariable receiver;
    core::NameRef method;
    u4 arg;

    LoadArg(core::LocalVariable receiver, core::NameRef method, u4 arg) : receiver(receiver), method(method), arg(arg) {
        core::categoryCounterInc("cfg", "loadarg");
    };
    virtual std::string toString(core::Context ctx);
};

class LoadYieldParams final : public Instruction {
public:
    std::shared_ptr<core::SendAndBlockLink> link;
    core::SymbolRef block;

    LoadYieldParams(std::shared_ptr<core::SendAndBlockLink> link, core::SymbolRef blk) : link(link), block(blk) {
        core::categoryCounterInc("cfg", "loadarg");
    };
    virtual std::string toString(core::Context ctx);
};

class Cast final : public Instruction {
public:
    core::LocalVariable value;
    std::shared_ptr<core::Type> type;
    core::NameRef cast;

    Cast(core::LocalVariable value, std::shared_ptr<core::Type> type, core::NameRef cast)
        : value(value), type(type), cast(cast) {}

    virtual std::string toString(core::Context ctx);
};

class DebugEnvironment final : public Instruction {
public:
    std::string str;
    core::GlobalState::AnnotationPos pos;

    DebugEnvironment(core::GlobalState::AnnotationPos pos);
    virtual std::string toString(core::Context ctx);
};

} // namespace cfg
} // namespace sorbet

#endif // SORBET_CFG_H

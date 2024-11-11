#include "values.hpp"

using namespace moss;

Value::Value(TypeKind kind, ustring name, Value *type) 
        : references(1), kind(kind), type(type), name(name), 
          attrs(new MemoryPool()), annotations{} {
}


Value *Value::get_attr(ustring name) {
    return attrs->load_name(name);
}

void Value::set_attr(ustring name, Value *v) {
    auto reg = attrs->get_free_reg();
    attrs->store(reg, v);
    attrs->store_name(reg, name);
}

std::ostream& ClassValue::debug(std::ostream& os) const {
    os << "Class(" << name; 
    bool first = true;
    for (auto s : supers) {
        if (first) {
            os << ", parents: {" << s->get_name();
            first = false;
        }
        else {
            os << ", " << s->get_name();
        }
    }
    if (!first)
        os << "}";
    os << ")[refs: " << references << "]";
    //os << *attrs;
    return os;
}

Value *BuiltIns::Type = new ClassValue("Type");

Value *BuiltIns::Int = new ClassValue("Int");
Value *BuiltIns::Float = new ClassValue("Float");
Value *BuiltIns::Bool = new ClassValue("Bool");
Value *BuiltIns::NilType = new ClassValue("NilType");
Value *BuiltIns::String = new ClassValue("String");
Value *BuiltIns::List = new ClassValue("List");

Value *BuiltIns::Address = new ClassValue("Address");

Value *BuiltIns::Function = new ClassValue("Function");
Value *BuiltIns::FunctionList = new ClassValue("FunctionList");

Value *BuiltIns::Nil = new NilValue();
Value *BuiltIns::IntConstants[BUILT_INS_INT_CONSTANTS_AM] = {
    new IntValue(0), new IntValue(1), new IntValue(2), new IntValue(3),
    new IntValue(4), new IntValue(5), new IntValue(6), new IntValue(7),
    new IntValue(8), new IntValue(9), new IntValue(10), new IntValue(11),
    new IntValue(12), new IntValue(13), new IntValue(14), new IntValue(15),
    new IntValue(16), new IntValue(17), new IntValue(18), new IntValue(19),
    new IntValue(20), new IntValue(21), new IntValue(22), new IntValue(23),
    new IntValue(24), new IntValue(25), new IntValue(26), new IntValue(27),
    new IntValue(28), new IntValue(29), new IntValue(30), new IntValue(31),
    new IntValue(32), new IntValue(33), new IntValue(34), new IntValue(35),
    new IntValue(36), new IntValue(37), new IntValue(38), new IntValue(39),
    new IntValue(40), new IntValue(41), new IntValue(42), new IntValue(43),
    new IntValue(44), new IntValue(45), new IntValue(46), new IntValue(47),
    new IntValue(48), new IntValue(49), new IntValue(50), new IntValue(51),
    new IntValue(52), new IntValue(53), new IntValue(54), new IntValue(55),
    new IntValue(56), new IntValue(57), new IntValue(58), new IntValue(59),
    new IntValue(60), new IntValue(61), new IntValue(62), new IntValue(63),
    new IntValue(64), new IntValue(65), new IntValue(66), new IntValue(67),
    new IntValue(68), new IntValue(69), new IntValue(70), new IntValue(71),
    new IntValue(72), new IntValue(73), new IntValue(74), new IntValue(75),
    new IntValue(76), new IntValue(77), new IntValue(78), new IntValue(79),
    new IntValue(80), new IntValue(81), new IntValue(82), new IntValue(83),
    new IntValue(84), new IntValue(85), new IntValue(86), new IntValue(87),
    new IntValue(88), new IntValue(89), new IntValue(90), new IntValue(91),
    new IntValue(92), new IntValue(93), new IntValue(94), new IntValue(95),
    new IntValue(96), new IntValue(97), new IntValue(98), new IntValue(99),
    new IntValue(100), new IntValue(101), new IntValue(102), new IntValue(103),
    new IntValue(104), new IntValue(105), new IntValue(106), new IntValue(107),
    new IntValue(108), new IntValue(109), new IntValue(110), new IntValue(111),
    new IntValue(112), new IntValue(113), new IntValue(114), new IntValue(115),
    new IntValue(116), new IntValue(117), new IntValue(118), new IntValue(119),
    new IntValue(120), new IntValue(121), new IntValue(122), new IntValue(123),
    new IntValue(124), new IntValue(125), new IntValue(126), new IntValue(127),
    new IntValue(128), new IntValue(129), new IntValue(130), new IntValue(131),
    new IntValue(132), new IntValue(133), new IntValue(134), new IntValue(135),
    new IntValue(136), new IntValue(137), new IntValue(138), new IntValue(139),
    new IntValue(140), new IntValue(141), new IntValue(142), new IntValue(143),
    new IntValue(144), new IntValue(145), new IntValue(146), new IntValue(147),
    new IntValue(148), new IntValue(149), new IntValue(150), new IntValue(151),
    new IntValue(152), new IntValue(153), new IntValue(154), new IntValue(155),
    new IntValue(156), new IntValue(157), new IntValue(158), new IntValue(159),
    new IntValue(160), new IntValue(161), new IntValue(162), new IntValue(163),
    new IntValue(164), new IntValue(165), new IntValue(166), new IntValue(167),
    new IntValue(168), new IntValue(169), new IntValue(170), new IntValue(171),
    new IntValue(172), new IntValue(173), new IntValue(174), new IntValue(175),
    new IntValue(176), new IntValue(177), new IntValue(178), new IntValue(179),
    new IntValue(180), new IntValue(181), new IntValue(182), new IntValue(183),
    new IntValue(184), new IntValue(185), new IntValue(186), new IntValue(187),
    new IntValue(188), new IntValue(189), new IntValue(190), new IntValue(191),
    new IntValue(192), new IntValue(193), new IntValue(194), new IntValue(195),
    new IntValue(196), new IntValue(197), new IntValue(198), new IntValue(199),
    new IntValue(200), new IntValue(201), new IntValue(202), new IntValue(203),
    new IntValue(204), new IntValue(205), new IntValue(206), new IntValue(207),
    new IntValue(208), new IntValue(209), new IntValue(210), new IntValue(211),
    new IntValue(212), new IntValue(213), new IntValue(214), new IntValue(215),
    new IntValue(216), new IntValue(217), new IntValue(218), new IntValue(219),
    new IntValue(220), new IntValue(221), new IntValue(222), new IntValue(223),
    new IntValue(224), new IntValue(225), new IntValue(226), new IntValue(227),
    new IntValue(228), new IntValue(229), new IntValue(230), new IntValue(231),
    new IntValue(232), new IntValue(233), new IntValue(234), new IntValue(235),
    new IntValue(236), new IntValue(237), new IntValue(238), new IntValue(239),
    new IntValue(240), new IntValue(241), new IntValue(242), new IntValue(243),
    new IntValue(244), new IntValue(245), new IntValue(246), new IntValue(247),
    new IntValue(248), new IntValue(249), new IntValue(250), new IntValue(251),
    new IntValue(252), new IntValue(253), new IntValue(254), new IntValue(255),
    new IntValue(256),
    new IntValue(-1), new IntValue(-2), new IntValue(-3), new IntValue(-4),
    new IntValue(-5)
};
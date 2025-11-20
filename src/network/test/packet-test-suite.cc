/*
 * Copyright (c) 2005,2006 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
/**
 * @file
 * @ingroup packet
 */

#include "ns3/packet-tag-list.h"
#include "ns3/packet.h"
#include "ns3/test.h"

#include <cstdarg>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <limits> // std:numeric_limits
#include <sstream>
#include <string>
#include <tuple>

using namespace ns3;

//-----------------------------------------------------------------------------
// Unit tests
//-----------------------------------------------------------------------------
namespace
{

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Base class for Test tags
 *
 * @note Class internal to packet-test-suite.cc
 */
class ATestTagBase : public Tag
{
  public:
    ATestTagBase()
        : m_error(false),
          m_data(0)
    {
    }

    /// Constructor
    /// @param data Tag data
    ATestTagBase(uint8_t data)
        : m_error(false),
          m_data(data)
    {
    }

    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid =
            TypeId("ATestTagBase").SetParent<Tag>().SetGroupName("Network").HideFromDocumentation()
            // No AddConstructor because this is an abstract class.
            ;
        return tid;
    }

    /// Get the tag data.
    /// @return the tag data.
    int GetData() const
    {
        int result = (int)m_data;
        return result;
    }

    bool m_error;   //!< Error in the Tag
    uint8_t m_data; //!< Tag data
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Template class for Test tags
 *
 * @note Class internal to packet-test-suite.cc
 */
template <int N>
class ATestTag : public ATestTagBase
{
  public:
    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        std::ostringstream oss;
        oss << "anon::ATestTag<" << N << ">";
        static TypeId tid = TypeId(oss.str())
                                .SetParent<ATestTagBase>()
                                .SetGroupName("Network")
                                .HideFromDocumentation()
                                .AddConstructor<ATestTag<N>>();
        return tid;
    }

    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    uint32_t GetSerializedSize() const override
    {
        return N + sizeof(m_data);
    }

    void Serialize(TagBuffer buf) const override
    {
        buf.WriteU8(m_data);
        for (uint32_t i = 0; i < N; ++i)
        {
            buf.WriteU8(N);
        }
    }

    void Deserialize(TagBuffer buf) override
    {
        m_data = buf.ReadU8();
        for (uint32_t i = 0; i < N; ++i)
        {
            uint8_t v = buf.ReadU8();
            if (v != N)
            {
                m_error = true;
            }
        }
    }

    void Print(std::ostream& os) const override
    {
        os << N << "(" << m_data << ")";
    }

    ATestTag()
        : ATestTagBase()
    {
    }

    /// Constructor
    /// @param data Tag data
    ATestTag(uint8_t data)
        : ATestTagBase(data)
    {
    }
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Template class for Large Test tags
 *
 * @see Bug 2221: Expanding packet tag maximum size
 *
 * @note Class internal to packet-test-suite.cc
 */
class ALargeTestTag : public Tag
{
    /**
     * Size of large tags.
     * Previous versions of ns-3 limited the tag size to 20 bytes or less,
     * prior value: `static const uint8_t LARGE_TAG_BUFFER_SIZE = 64;`
     */
    static constexpr uint8_t LARGE_TAG_BUFFER_SIZE{64};

  public:
    ALargeTestTag()
    {
        for (uint8_t i = 0; i < (LARGE_TAG_BUFFER_SIZE - 1); i++)
        {
            m_data.push_back(i);
        }
        m_size = LARGE_TAG_BUFFER_SIZE;
    }

    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ALargeTestTag")
                                .SetParent<Tag>()
                                .SetGroupName("Network")
                                .HideFromDocumentation()
                                .AddConstructor<ALargeTestTag>();
        return tid;
    }

    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    uint32_t GetSerializedSize() const override
    {
        return (uint32_t)m_size;
    }

    void Serialize(TagBuffer buf) const override
    {
        buf.WriteU8(m_size);
        for (uint8_t i = 0; i < (m_size - 1); ++i)
        {
            buf.WriteU8(m_data[i]);
        }
    }

    void Deserialize(TagBuffer buf) override
    {
        m_size = buf.ReadU8();
        for (uint8_t i = 0; i < (m_size - 1); ++i)
        {
            uint8_t v = buf.ReadU8();
            m_data.push_back(v);
        }
    }

    void Print(std::ostream& os) const override
    {
        os << "(" << (uint16_t)m_size << ")";
    }

  private:
    uint8_t m_size;              //!< Packet size
    std::vector<uint8_t> m_data; //!< Tag data
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Base class for Test headers
 *
 * @note Class internal to packet-test-suite.cc
 */
class ATestHeaderBase : public Header
{
  public:
    ATestHeaderBase()
        : Header(),
          m_error(false)
    {
    }

    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ATestHeaderBase")
                                .SetParent<Header>()
                                .SetGroupName("Network")
                                .HideFromDocumentation()
            // No AddConstructor because this is an abstract class.
            ;
        return tid;
    }

    bool m_error; //!< Error in the Header
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Template class for Test headers
 *
 * @note Class internal to packet-test-suite.cc
 */
template <int N>
class ATestHeader : public ATestHeaderBase
{
  public:
    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        std::ostringstream oss;
        oss << "anon::ATestHeader<" << N << ">";
        static TypeId tid = TypeId(oss.str())
                                .SetParent<ATestHeaderBase>()
                                .SetGroupName("Network")
                                .HideFromDocumentation()
                                .AddConstructor<ATestHeader<N>>();
        return tid;
    }

    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    uint32_t GetSerializedSize() const override
    {
        return N;
    }

    void Serialize(Buffer::Iterator iter) const override
    {
        for (uint32_t i = 0; i < N; ++i)
        {
            iter.WriteU8(N);
        }
    }

    uint32_t Deserialize(Buffer::Iterator iter) override
    {
        for (uint32_t i = 0; i < N; ++i)
        {
            uint8_t v = iter.ReadU8();
            if (v != N)
            {
                m_error = true;
            }
        }
        return N;
    }

    void Print(std::ostream& os) const override
    {
    }

    ATestHeader()
        : ATestHeaderBase()
    {
    }
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Base class for Test trailers
 *
 * @note Class internal to packet-test-suite.cc
 */
class ATestTrailerBase : public Trailer
{
  public:
    ATestTrailerBase()
        : Trailer(),
          m_error(false)
    {
    }

    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        static TypeId tid = TypeId("ATestTrailerBase")
                                .SetParent<Trailer>()
                                .SetGroupName("Network")
                                .HideFromDocumentation()
            // No AddConstructor because this is an abstract class.
            ;
        return tid;
    }

    bool m_error; //!< Error in the Trailer
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Template class for Test trailers
 *
 * @note Class internal to packet-test-suite.cc
 */
template <int N>
class ATestTrailer : public ATestTrailerBase
{
  public:
    /**
     * Register this type.
     * @return The TypeId.
     */
    static TypeId GetTypeId()
    {
        std::ostringstream oss;
        oss << "anon::ATestTrailer<" << N << ">";
        static TypeId tid = TypeId(oss.str())
                                .SetParent<ATestTrailerBase>()
                                .SetGroupName("Network")
                                .HideFromDocumentation()
                                .AddConstructor<ATestTrailer<N>>();
        return tid;
    }

    TypeId GetInstanceTypeId() const override
    {
        return GetTypeId();
    }

    uint32_t GetSerializedSize() const override
    {
        return N;
    }

    void Serialize(Buffer::Iterator iter) const override
    {
        iter.Prev(N);
        for (uint32_t i = 0; i < N; ++i)
        {
            iter.WriteU8(N);
        }
    }

    uint32_t Deserialize(Buffer::Iterator iter) override
    {
        iter.Prev(N);
        for (uint32_t i = 0; i < N; ++i)
        {
            uint8_t v = iter.ReadU8();
            if (v != N)
            {
                m_error = true;
            }
        }
        return N;
    }

    void Print(std::ostream& os) const override
    {
    }

    ATestTrailer()
        : ATestTrailerBase()
    {
    }
};

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Struct to hold the expected data in the packet
 *
 * @note Class internal to packet-test-suite.cc
 */
struct Expected
{
    /**
     * Constructor
     * @param n_ Number of elements
     * @param start_ Start
     * @param end_ End
     */
    Expected(uint32_t n_, uint32_t start_, uint32_t end_)
        : n(n_),
          start(start_),
          end(end_),
          data(0)
    {
    }

    /**
     * Constructor
     * @param n_ Number of elements
     * @param start_ Start
     * @param end_ End
     * @param data_ Data stored in tag
     */
    Expected(uint32_t n_, uint32_t start_, uint32_t end_, uint8_t data_)
        : n(n_),
          start(start_),
          end(end_),
          data(data_)
    {
    }

    uint32_t n;     //!< Number of elements
    uint32_t start; //!< Start
    uint32_t end;   //!< End
    uint8_t data;   //!< Optional data
};

} // namespace

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * Packet unit tests.
 */
class PacketTest : public TestCase
{
  public:
    PacketTest();
    void DoRun() override;

  private:
    /**
     * Checks the packet
     * @param p The packet
     * @param eTags List of Expected tags
     * @param ... The variable arguments
     */
    void DoCheck(Ptr<const Packet> p, std::initializer_list<Expected> eTags = {});
    /**
     * Checks the packet and its data
     * @param p The packet
     * @param eTags List of Expected tags
     * @param ... The variable arguments
     */
    void DoCheckData(Ptr<const Packet> p, std::initializer_list<Expected> eTags);
};

PacketTest::PacketTest()
    : TestCase("Packet: ")
{
}

void
PacketTest::DoCheck(Ptr<const Packet> p, std::initializer_list<Expected> eTags /* = {} */)
{
    static int testcount{0};
    ++testcount;

    std::vector<Expected> expected(eTags.begin(), eTags.end());

    ByteTagIterator i = p->GetByteTagIterator();
    uint32_t j = 0;
    while (i.HasNext() && j < expected.size())
    {
        ByteTagIterator::Item item = i.Next();
        Expected e = expected[j];
        std::ostringstream oss;
        oss << "anon::ATestTag<" << e.n << ">";
        NS_TEST_EXPECT_MSG_EQ(item.GetTypeId().GetName(), oss.str(), "trivial");
        NS_TEST_EXPECT_MSG_EQ(item.GetStart(), e.start, "trivial");
        NS_TEST_EXPECT_MSG_EQ(item.GetEnd(), e.end, "trivial");
        ATestTagBase* tag = dynamic_cast<ATestTagBase*>(item.GetTypeId().GetConstructor()());
        NS_TEST_EXPECT_MSG_NE(tag, 0, "trivial");
        item.GetTag(*tag);
        NS_TEST_EXPECT_MSG_EQ(tag->m_error, false, "trivial");
        delete tag;
        j++;
    }
    NS_TEST_EXPECT_MSG_EQ(i.HasNext(), false, testcount << " Nothing left");
    NS_TEST_EXPECT_MSG_EQ(j, expected.size(), testcount << " Size match");
}

void
PacketTest::DoCheckData(Ptr<const Packet> p, std::initializer_list<Expected> eTags)
{
    static int testcount{0};
    ++testcount;

    std::vector<Expected> expected(eTags.begin(), eTags.end());

    ByteTagIterator i = p->GetByteTagIterator();
    uint32_t j = 0;
    while (i.HasNext() && j < expected.size())
    {
        ByteTagIterator::Item item = i.Next();
        Expected e = expected[j];
        std::ostringstream oss;
        oss << "anon::ATestTag<" << e.n << ">";
        NS_TEST_EXPECT_MSG_EQ(item.GetTypeId().GetName(), oss.str(), "trivial");
        NS_TEST_EXPECT_MSG_EQ(item.GetStart(), e.start, "trivial");
        NS_TEST_EXPECT_MSG_EQ(item.GetEnd(), e.end, "trivial");
        ATestTagBase* tag = dynamic_cast<ATestTagBase*>(item.GetTypeId().GetConstructor()());
        NS_TEST_EXPECT_MSG_NE(tag, 0, "trivial");
        item.GetTag(*tag);
        NS_TEST_EXPECT_MSG_EQ(tag->m_error, false, "trivial");
        NS_TEST_EXPECT_MSG_EQ(tag->GetData(), e.data, "trivial");
        delete tag;
        j++;
    }
    NS_TEST_EXPECT_MSG_EQ(i.HasNext(), false, testcount << " Nothing left");
    NS_TEST_EXPECT_MSG_EQ(j, expected.size(), testcount << " Size match");
}

void
PacketTest::DoRun()
{
    Ptr<Packet> pkt1 = Create<Packet>(reinterpret_cast<const uint8_t*>("hello"), 5);
    Ptr<Packet> pkt2 = Create<Packet>(reinterpret_cast<const uint8_t*>(" world"), 6);
    Ptr<Packet> packet = Create<Packet>();

    std::cout << GetName() << "adding packets at end" << std::endl;
    packet->AddAtEnd(pkt1);
    packet->AddAtEnd(pkt2);

    NS_TEST_EXPECT_MSG_EQ(packet->GetSize(), 11, "trivial");

    std::cout << GetName() << "copying packet data" << std::endl;
    auto buf = new uint8_t[packet->GetSize()];
    packet->CopyData(buf, packet->GetSize());

    std::string msg = std::string(reinterpret_cast<const char*>(buf), packet->GetSize());
    delete[] buf;

    std::cout << GetName() << "expecting 'hello world' from adding packets at end" << std::endl;
    NS_TEST_EXPECT_MSG_EQ(msg, "hello world", "trivial");

    Ptr<const Packet> p = Create<Packet>(1000);

    std::cout << GetName() << "byte tags: add" << std::endl;
    p->AddByteTag(ATestTag<1>());
    DoCheck(p, {{1, 0, 1000}});
    std::cout << GetName() << "byte tags: copy" << std::endl;
    Ptr<const Packet> copy = p->Copy();
    DoCheck(copy, {{1, 0, 1000}});

    std::cout << GetName() << "byte tags: add 2nd" << std::endl;
    p->AddByteTag(ATestTag<2>());
    std::cout << GetName() << "byte tags: unmodified original" << std::endl;
    DoCheck(p, {{1, 0, 1000}, {2, 0, 1000}});
    DoCheck(copy, {{1, 0, 1000}});

    {
        std::cout << GetName() << "assignment" << std::endl;
        Packet c0 = *copy;
        Packet c1 = *copy; // NOLINT(performance-unnecessary-copy-initialization)
        c0 = c1;
        DoCheck(&c0, {{1, 0, 1000}});
        DoCheck(&c1, {{1, 0, 1000}});
        DoCheck(copy, {{1, 0, 1000}});
        c0.AddByteTag(ATestTag<10>());
        DoCheck(&c0, {{1, 0, 1000}, {10, 0, 1000}});
        DoCheck(&c1, {{1, 0, 1000}});
        DoCheck(copy, {{1, 0, 1000}});
    }

    std::cout << GetName() << "bytetags: fragmentation" << std::endl;
    Ptr<Packet> frag0 = p->CreateFragment(0, 10);
    Ptr<Packet> frag1 = p->CreateFragment(10, 90);
    Ptr<const Packet> frag2 = p->CreateFragment(100, 900);
    frag0->AddByteTag(ATestTag<3>());
    DoCheck(frag0, {{1, 0, 10}, {2, 0, 10}, {3, 0, 10}});
    frag1->AddByteTag(ATestTag<4>());
    DoCheck(frag1, {{1, 0, 90}, {2, 0, 90}, {4, 0, 90}});
    frag2->AddByteTag(ATestTag<5>());
    DoCheck(frag2, {{1, 0, 900}, {2, 0, 900}, {5, 0, 900}});

    frag1->AddAtEnd(frag2);
    DoCheck(frag1, {{1, 0, 90}, {2, 0, 90}, {4, 0, 90}, {1, 90, 990}, {2, 90, 990}, {5, 90, 990}});

    DoCheck(frag0, {{1, 0, 10}, {2, 0, 10}, {3, 0, 10}});
    frag0->AddAtEnd(frag1);
    DoCheck(frag0,
            {{1, 0, 10},
             {2, 0, 10},
             {3, 0, 10},
             {1, 10, 100},
             {2, 10, 100},
             {4, 10, 100},
             {1, 100, 1000},
             {2, 100, 1000},
             {5, 100, 1000}});

    // force caching a buffer of the right size.
    std::cout << GetName() << "right-sized buffer" << std::endl;
    frag0 = Create<Packet>(1000);
    frag0->AddHeader(ATestHeader<10>());
    frag0 = nullptr;

    p = Create<Packet>(1000);
    p->AddByteTag(ATestTag<20>());
    DoCheck(p, {{20, 0, 1000}});
    frag0 = p->CreateFragment(10, 90);
    DoCheck(p, {{20, 0, 1000}});
    DoCheck(frag0, {{20, 0, 90}});
    p = nullptr;
    frag0->AddHeader(ATestHeader<10>());
    DoCheck(frag0, {{20, 10, 100}});

    {
        std::cout << GetName() << "add/remove header" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(100);
        tmp->AddByteTag(ATestTag<20>());
        DoCheck(tmp, {{20, 0, 100}});
        tmp->AddHeader(ATestHeader<10>());
        DoCheck(tmp, {{20, 10, 110}});
        ATestHeader<10> h;
        tmp->RemoveHeader(h);
        DoCheck(tmp, {{20, 0, 100}});
        tmp->AddHeader(ATestHeader<10>());
        DoCheck(tmp, {{20, 10, 110}});

        std::cout << GetName() << "add/remove trailer" << std::endl;
        tmp = Create<Packet>(100);
        tmp->AddByteTag(ATestTag<20>());
        DoCheck(tmp, {{20, 0, 100}});
        tmp->AddTrailer(ATestTrailer<10>());
        DoCheck(tmp, {{20, 0, 100}});
        ATestTrailer<10> t;
        tmp->RemoveTrailer(t);
        DoCheck(tmp, {{20, 0, 100}});
        tmp->AddTrailer(ATestTrailer<10>());
        DoCheck(tmp, {{20, 0, 100}});
    }

    {
        std::cout << GetName() << "empty packet, remove at start/add at end" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddHeader(ATestHeader<156>());
        tmp->AddByteTag(ATestTag<20>());
        DoCheck(tmp, {{20, 0, 156}});
        tmp->RemoveAtStart(120);
        DoCheck(tmp, {{20, 0, 36}});
        Ptr<Packet> a = Create<Packet>(0);
        a->AddAtEnd(tmp);
        DoCheck(a, {{20, 0, 36}});
    }

    {
        std::cout << GetName() << "empty packet" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddByteTag(ATestTag<20>());
        DoCheck(tmp); // Adding byte tag beyond end is a no-op
    }
    {
        std::cout << GetName() << "sized packet, remove at start/add at end" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(1000);
        tmp->AddByteTag(ATestTag<20>());
        DoCheck(tmp, {{20, 0, 1000}});
        tmp->RemoveAtStart(1000);
        DoCheck(tmp); // RemoveAtStart also removes byte tag
        Ptr<Packet> a = Create<Packet>(10);
        a->AddByteTag(ATestTag<10>());
        DoCheck(a, {{10, 0, 10}});
        tmp->AddAtEnd(a);
        DoCheck(tmp, {{10, 0, 10}});
    }

    {
        std::cout << GetName() << "peek packet tags" << std::endl;
        Packet p;
        ATestTag<10> a;
        p.AddPacketTag(a);
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(a), true, "trivial");
        ATestTag<11> b;
        p.AddPacketTag(b);
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(b), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(a), true, "trivial");
        Packet copy = p;
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(b), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(a), true, "trivial");
        ATestTag<12> c;
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(c), false, "trivial");
        copy.AddPacketTag(c);
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(c), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(b), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(a), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(c), false, "trivial");
        copy.RemovePacketTag(b);
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(b), false, "trivial");
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(b), true, "trivial");
        p.RemovePacketTag(a);
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(a), false, "trivial");
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(a), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(c), false, "trivial");
        NS_TEST_EXPECT_MSG_EQ(copy.PeekPacketTag(c), true, "trivial");
        p.RemoveAllPacketTags();
        NS_TEST_EXPECT_MSG_EQ(p.PeekPacketTag(b), false, "trivial");
    }

    /* Test Serialization and Deserialization of Packet with PacketTag data */
    {
        std::cout << GetName() << "serialization/deserialization with PacketTag data" << std::endl;
        Ptr<Packet> p1 = Create<Packet>(1000);
        ;
        ATestTag<10> a1(65);
        ATestTag<11> b1(66);
        ATestTag<12> c1(67);

        p1->AddPacketTag(a1);
        p1->AddPacketTag(b1);
        p1->AddPacketTag(c1);

        uint32_t serializedSize = p1->GetSerializedSize();
        auto buffer = new uint8_t[serializedSize + 16];
        p1->Serialize(buffer, serializedSize);

        Ptr<Packet> p2 = Create<Packet>(buffer, serializedSize, true);

        delete[] buffer;

        ATestTag<10> a2;
        ATestTag<11> b2;
        ATestTag<12> c2;

        NS_TEST_EXPECT_MSG_EQ(p2->PeekPacketTag(a2), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(a2.GetData(), 65, "trivial");
        NS_TEST_EXPECT_MSG_EQ(p2->PeekPacketTag(b2), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(b2.GetData(), 66, "trivial");
        NS_TEST_EXPECT_MSG_EQ(p2->PeekPacketTag(c2), true, "trivial");
        NS_TEST_EXPECT_MSG_EQ(c2.GetData(), 67, "trivial");
    }

    /* Test Serialization and Deserialization of Packet with ByteTag data */
    {
        std::cout << GetName() << "serialization/deserialization with ByteTag data" << std::endl;
        Ptr<Packet> p1 = Create<Packet>(1000);
        ;

        ATestTag<10> a1(65);
        ATestTag<11> b1(66);
        ATestTag<12> c1(67);

        p1->AddByteTag(a1);
        p1->AddByteTag(b1);
        p1->AddByteTag(c1);

        DoCheck(p1, {{10, 0, 1000}, {11, 0, 1000}, {12, 0, 1000}});

        uint32_t serializedSize = p1->GetSerializedSize();
        auto buffer = new uint8_t[serializedSize];
        p1->Serialize(buffer, serializedSize);

        Ptr<Packet> p2 = Create<Packet>(buffer, serializedSize, true);

        delete[] buffer;

        DoCheckData(p2, {{10, 0, 1000, 65}, {11, 0, 1000, 66}, {12, 0, 1000, 67}});
    }

    {
        std::cout << GetName() << "bug 572" << std::endl;
        /// @internal
        /// See \bugid{572}
        Ptr<Packet> tmp = Create<Packet>(1000);
        tmp->AddByteTag(ATestTag<20>());
        DoCheck(tmp, {{20, 0, 1000}});
        tmp->AddHeader(ATestHeader<2>());
        DoCheck(tmp, {{20, 2, 1002}});
        tmp->RemoveAtStart(1);
        DoCheck(tmp, {{20, 1, 1001}});
#if 0
    tmp->PeekData ();
    DoCheck(tmp, {{20, 1, 1001}});
#endif
    }

    /* Test reducing tagged packet size and increasing it back. */
    {
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddHeader(ATestHeader<100>());
        tmp->AddByteTag(ATestTag<25>());
        DoCheck(tmp, {{25, 0, 100}});
        tmp->RemoveAtStart(50);
        DoCheck(tmp, {{25, 0, 50}});
        tmp->AddHeader(ATestHeader<50>());
        DoCheck(tmp, {{25, 50, 100}});
    }

    /* Similar test case, but using trailer instead of header. */
    {
        std::cout << GetName() << "reduce/increase size using trailer" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddTrailer(ATestTrailer<100>());
        tmp->AddByteTag(ATestTag<25>());
        DoCheck(tmp, {{25, 0, 100}});
        tmp->RemoveAtEnd(50);
        DoCheck(tmp, {{25, 0, 50}});
        tmp->AddTrailer(ATestTrailer<50>());
        DoCheck(tmp, {{25, 0, 50}});
    }

    /* Test reducing tagged packet size and increasing it by half. */
    {
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddHeader(ATestHeader<100>());
        tmp->AddByteTag(ATestTag<25>());
        DoCheck(tmp, {{25, 0, 100}});
        tmp->RemoveAtStart(50);
        DoCheck(tmp, {{25, 0, 50}});
        tmp->AddHeader(ATestHeader<25>());
        DoCheck(tmp, {{25, 25, 75}});
    }

    /* Similar test case, but using trailer instead of header. */
    {
        std::cout << GetName() << "reduce/increase trailer size in presence of byte tag"
                  << std::endl;
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddTrailer(ATestTrailer<100>());
        tmp->AddByteTag(ATestTag<25>());
        DoCheck(tmp, {{25, 0, 100}});
        tmp->RemoveAtEnd(50);
        DoCheck(tmp, {{25, 0, 50}});
        tmp->AddTrailer(ATestTrailer<25>());
        DoCheck(tmp, {{25, 0, 50}});
    }

    /* Test AddPaddingAtEnd. */
    {
        std::cout << GetName() << "add/remove trailer then padding" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(0);
        tmp->AddTrailer(ATestTrailer<100>());
        tmp->AddByteTag(ATestTag<25>());
        DoCheck(tmp, {{25, 0, 100}});
        tmp->RemoveAtEnd(50);
        DoCheck(tmp, {{25, 0, 50}});
        tmp->AddPaddingAtEnd(50);
        DoCheck(tmp, {{25, 0, 50}});
    }

    /* Test reducing tagged packet size and increasing it back,
     * now using padding bytes to avoid triggering dirty state
     * in virtual buffer
     */
    {
        std::cout << GetName() << "remove/add padding at end" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(100);
        tmp->AddByteTag(ATestTag<25>());
        DoCheck(tmp, {{25, 0, 100}});
        tmp->RemoveAtEnd(50);
        DoCheck(tmp, {{25, 0, 50}});
        tmp->AddPaddingAtEnd(50);
        DoCheck(tmp, {{25, 0, 50}});
    }

    /* Test ALargeTestTag */
    {
        std::cout << GetName() << "large packet tag" << std::endl;
        Ptr<Packet> tmp = Create<Packet>(0);
        ALargeTestTag a;
        tmp->AddPacketTag(a);
    }
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * Packet Tag list unit tests.
 */
class PacketTagListTest : public TestCase
{
  public:
    PacketTagListTest();
    ~PacketTagListTest() override;

  private:
    void DoRun() override;
    /**
     * Checks against a reference PacketTagList
     * @param ref Reference
     * @param t List to test
     * @param msg Message
     * @param miss Expected miss/hit
     */
    void CheckRef(const PacketTagList& ref,
                  ATestTagBase& t,
                  const std::string& msg,
                  bool miss = false);
    /**
     * Checks against a reference PacketTagList
     * @param ref Reference packet tag list
     * @param msg Message
     * @param miss Expected miss/hit
     */
    void CheckRefList(const PacketTagList& ref, const std::string& msg, int miss = 0);

    /** Number of test tags returned by MakeTestTags */
    static constexpr auto TAG_LAST{7};

    /**n
     * Create a set of tags to be used in a series of tests
     * @return an array of test tags
     * Example usage:
     *     auto [t1, t2, t3, t4, t5, t6, t7] = MakeTestTags();
     */
    std::tuple<ATestTag<1>,
               ATestTag<2>,
               ATestTag<3>,
               ATestTag<4>,
               ATestTag<5>,
               ATestTag<6>,
               ATestTag<7>>
    MakeTestTags();
    /**
     * Check removal of a tag
     * @param ref Reference PacketTagList
     * @param tag The tag
     * @param miss Expected miss/hit
     */
    void RemoveCheck(const PacketTagList& ref, ATestTagBase& tag, int miss = 0);
    /**
     * Check replacement of a tag
     * @copydetails RemoveCheck()
     */
    void ReplaceCheck(const PacketTagList& ref, ATestTagBase& tag, int miss = 0);

    /**
     * Prints the remove time
     * @param ref Reference.
     * @param t List to test.
     * @param msg Message - prints on cout if msg is not null.
     * @return the ticks to remove the tags.
     */
    int RemoveTime(const PacketTagList& ref, ATestTagBase& t, const std::string& msg = "");

    /**
     * Prints the remove time
     * @param verbose prints on cout if verbose is true.
     * @return the ticks to remove the tags.
     */
    int AddRemoveTime(const bool verbose = false);
};

PacketTagListTest::PacketTagListTest()
    : TestCase("PacketTagListTest: ")
{
}

PacketTagListTest::~PacketTagListTest()
{
}

void
PacketTagListTest::CheckRef(const PacketTagList& ref,
                            ATestTagBase& t,
                            const std::string& msg,
                            bool miss)
{
    int expect = t.GetData(); // the value we should find
    bool found = ref.Peek(t); // rewrites t with actual value
    NS_TEST_EXPECT_MSG_EQ(found, !miss, msg << ": ref contains " << t.GetTypeId().GetName());
    if (found)
    {
        NS_TEST_EXPECT_MSG_EQ(t.GetData(),
                              expect,
                              msg << ": ref " << t.GetTypeId().GetName() << " = " << expect);
    }
}

/* static */
std::
    tuple<ATestTag<1>, ATestTag<2>, ATestTag<3>, ATestTag<4>, ATestTag<5>, ATestTag<6>, ATestTag<7>>
    PacketTagListTest::MakeTestTags()
{
    return {ATestTag<1>(),
            ATestTag<2>(),
            ATestTag<3>(),
            ATestTag<4>(),
            ATestTag<5>(),
            ATestTag<6>(),
            ATestTag<7>()};
}

void
PacketTagListTest::CheckRefList(const PacketTagList& ptl,
                                const std::string& msg,
                                int miss /* = 0 */)
{
    auto [t1, t2, t3, t4, t5, t6, t7] = MakeTestTags();
    CheckRef(ptl, t1, msg, miss == 1);
    CheckRef(ptl, t2, msg, miss == 2);
    CheckRef(ptl, t3, msg, miss == 3);
    CheckRef(ptl, t4, msg, miss == 4);
    CheckRef(ptl, t5, msg, miss == 5);
    CheckRef(ptl, t6, msg, miss == 6);
    CheckRef(ptl, t7, msg, miss == 7);
}

int
PacketTagListTest::RemoveTime(const PacketTagList& ref,
                              ATestTagBase& t,
                              const std::string& msg /* = "" */)
{
    const int reps = 10000;
    std::vector<PacketTagList> ptv(reps, ref);
    int start = clock();
    for (int i = 0; i < reps; ++i)
    {
        ptv[i].Remove(t);
    }
    int stop = clock();
    int delta = stop - start;
    if (!msg.empty())
    {
        std::cout << GetName() << "remove time: " << msg << ": " << std::setw(8) << delta
                  << " ticks to remove " << reps << " times" << std::endl;
    }
    return delta;
}

int
PacketTagListTest::AddRemoveTime(const bool verbose /* = false */)
{
    const int reps = 100000;
    PacketTagList ptl;
    ATestTag<2> t(2);
    int start = clock();
    for (int i = 0; i < reps; ++i)
    {
        ptl.Add(t);
        ptl.Remove(t);
    }
    int stop = clock();
    int delta = stop - start;
    if (verbose)
    {
        std::cout << GetName() << "add/remove time: " << std::setw(8) << delta
                  << " ticks to add+remove " << reps << " times" << std::endl;
    }
    return delta;
}

void
PacketTagListTest::RemoveCheck(const PacketTagList& ref, ATestTagBase& tag, int miss /* = 0 */)
{
    std::stringstream msg("remove ");
    msg << miss;

    PacketTagList ptl(ref);
    ptl.Remove(tag);
    CheckRefList(ref, msg.str() + " orig");
    CheckRefList(ptl, msg.str() + " copy", miss);
}

void
PacketTagListTest::ReplaceCheck(const PacketTagList& ref, ATestTagBase& tag, int miss /* = 0 */)
{
    std::stringstream msg("replace ");
    msg << miss;

    tag.m_data = 2;
    PacketTagList ptl = ref;
    ptl.Replace(tag);
    CheckRefList(ref, msg.str() + " orig");
    CheckRef(ptl, tag, msg.str() + " copy");
}

void
PacketTagListTest::DoRun()
{
    std::cout << GetName() << "begin" << std::endl;

    auto [t1, t2, t3, t4, t5, t6, t7] = MakeTestTags();

    PacketTagList ref; // empty list
    ref.Add(t1);       // last
    ref.Add(t2);       // post merge
    ref.Add(t3);       // merge successor
    ref.Add(t4);       // merge
    ref.Add(t5);       // merge precursor
    ref.Add(t6);       // pre-merge
    ref.Add(t7);       // first

    // Peek
    {
        std::cout << GetName() << "check Peek (missing tag) returns false" << std::endl;
        ATestTag<10> t10;
        NS_TEST_EXPECT_MSG_EQ(ref.Peek(t10), false, "missing tag");
    }

    // Copy ctor, assignment
    {
        std::cout << GetName() << "check copy and assignment" << std::endl;
        {
            // Test copy constructor
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            PacketTagList ptl(ref);
            CheckRefList(ref, "copy ctor orig");
            CheckRefList(ptl, "copy ctor copy");
        }
        {
            // Test copy constructor
            // NOLINTNEXTLINE(performance-unnecessary-copy-initialization)
            PacketTagList ptl = ref;
            CheckRefList(ref, "assignment orig");
            CheckRefList(ptl, "assignment copy");
        }
    }

    // Removal
    {
        // Remove single tags from list
        {
            std::cout << GetName() << "check removal of each tag" << std::endl;
            RemoveCheck(ref, t1, 1);
            RemoveCheck(ref, t2, 2);
            RemoveCheck(ref, t3, 3);
            RemoveCheck(ref, t4, 4);
            RemoveCheck(ref, t5, 5);
            RemoveCheck(ref, t6, 6);
            RemoveCheck(ref, t7, 7);
        }

        // Remove in the presence of a merge
        {
            std::cout << GetName() << "check removal doesn't disturb merge " << std::endl;
            PacketTagList ptl = ref;
            ptl.Remove(t7);
            ptl.Remove(t6);
            ptl.Remove(t5);

            PacketTagList mrg = ptl; // merged list
            ATestTag<8> m5(1);
            mrg.Add(m5); // ptl and mrg differ
            ptl.Add(t5);
            ptl.Add(t6);
            ptl.Add(t7);

            CheckRefList(ref, "post merge, orig");
            CheckRefList(ptl, "post merge, long chain");
            const char* msg = "post merge, short chain";
            CheckRef(mrg, t1, msg, false);
            CheckRef(mrg, t2, msg, false);
            CheckRef(mrg, t3, msg, false);
            CheckRef(mrg, t4, msg, false);
            CheckRef(mrg, m5, msg, false);
        }
    }

    // Replace
    {
        std::cout << GetName() << "check replacing each tag" << std::endl;
        ReplaceCheck(ref, t1, 1);
        ReplaceCheck(ref, t2, 2);
        ReplaceCheck(ref, t3, 3);
        ReplaceCheck(ref, t4, 4);
        ReplaceCheck(ref, t5, 5);
        ReplaceCheck(ref, t6, 6);
        ReplaceCheck(ref, t7, 7);
    }

    // Timing
    {
        std::cout << GetName() << "add+remove timing" << std::endl;
        int flm = std::numeric_limits<int>::max();
        const int nIterations = 100;
        for (int i = 0; i < nIterations; ++i)
        {
            int now = AddRemoveTime();
            if (now < flm)
            {
                flm = now;
            }
        }
        std::cout << GetName() << "min add+remove time: " << std::setw(8) << flm << " ticks"
                  << std::endl;

        std::cout << GetName() << "remove timing" << std::endl;
        // tags numbered from 1, so add one for (unused) entry at 0
        std::vector<int> rmn(TAG_LAST + 1, std::numeric_limits<int>::max());
        for (int i = 0; i < nIterations; ++i)
        {
            for (int j = 1; j <= TAG_LAST; ++j)
            {
                int now = 0;
                switch (j)
                {
                case 7:
                    now = RemoveTime(ref, t7);
                    break;
                case 6:
                    now = RemoveTime(ref, t6);
                    break;
                case 5:
                    now = RemoveTime(ref, t5);
                    break;
                case 4:
                    now = RemoveTime(ref, t4);
                    break;
                case 3:
                    now = RemoveTime(ref, t3);
                    break;
                case 2:
                    now = RemoveTime(ref, t2);
                    break;
                case 1:
                    now = RemoveTime(ref, t1);
                    break;
                }

                if (now < rmn[j])
                {
                    rmn[j] = now;
                }
            }
        }
        for (int j = TAG_LAST; j > 0; --j)
        {
            std::cout << GetName() << "min remove time: t" << j << ": " << std::setw(8) << rmn[j]
                      << " ticks" << std::endl;
        }
    }
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Packet TestSuite
 */
class PacketTestSuite : public TestSuite
{
  public:
    PacketTestSuite();
};

PacketTestSuite::PacketTestSuite()
    : TestSuite("packet", Type::UNIT)
{
    AddTestCase(new PacketTest, TestCase::Duration::QUICK);
    AddTestCase(new PacketTagListTest, TestCase::Duration::QUICK);
}

static PacketTestSuite g_packetTestSuite; //!< Static variable for test initialization

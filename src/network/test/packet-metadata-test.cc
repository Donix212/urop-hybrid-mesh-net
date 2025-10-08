/*
 * Copyright (c) 2006,2007 INRIA
 *
 * SPDX-License-Identifier: GPL-2.0-only
 *
 * Author: Mathieu Lacage <mathieu.lacage@sophia.inria.fr>
 */
#include "ns3/header.h"
#include "ns3/packet-metadata.h"
#include "ns3/packet.h"
#include "ns3/test.h"
#include "ns3/trailer.h"

#include <cstdarg>
#include <iostream>
#include <sstream>

using namespace ns3;

namespace
{

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Base header-type class to check the proper header concatenation
 *
 * @note Class internal to packet-metadata-test.cc
 */
class HistoryHeaderBase : public Header
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();
    HistoryHeaderBase();
    /**
     * Checks if the header has deserialization errors
     * @returns True if no error found.
     */
    bool IsOk() const;

  protected:
    /**
     * Signal that an error has been found in deserialization.
     */
    void ReportError();

  private:
    bool m_ok; //!< True if no error is signalled.
};

TypeId
HistoryHeaderBase::GetTypeId()
{
    static TypeId tid = TypeId("ns3::HistoryHeaderBase").SetParent<Header>();
    return tid;
}

HistoryHeaderBase::HistoryHeaderBase()
    : m_ok(true)
{
}

bool
HistoryHeaderBase::IsOk() const
{
    return m_ok;
}

void
HistoryHeaderBase::ReportError()
{
    m_ok = false;
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Template header-type class to check the proper header concatenation
 *
 * @note Class internal to packet-metadata-test.cc
 */
template <int N>
class HistoryHeader : public HistoryHeaderBase
{
  public:
    HistoryHeader();
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
};

template <int N>
HistoryHeader<N>::HistoryHeader()
    : HistoryHeaderBase()
{
}

template <int N>
TypeId
HistoryHeader<N>::GetTypeId()
{
    std::ostringstream oss;
    oss << "ns3::HistoryHeader<" << N << ">";
    static TypeId tid =
        TypeId(oss.str()).SetParent<HistoryHeaderBase>().AddConstructor<HistoryHeader<N>>();
    return tid;
}

template <int N>
TypeId
HistoryHeader<N>::GetInstanceTypeId() const
{
    return GetTypeId();
}

template <int N>
void
HistoryHeader<N>::Print(std::ostream& os) const
{
    NS_ASSERT(false);
}

template <int N>
uint32_t
HistoryHeader<N>::GetSerializedSize() const
{
    return N;
}

template <int N>
void
HistoryHeader<N>::Serialize(Buffer::Iterator start) const
{
    start.WriteU8(N, N);
}

template <int N>
uint32_t
HistoryHeader<N>::Deserialize(Buffer::Iterator start)
{
    for (int i = 0; i < N; i++)
    {
        if (start.ReadU8() != N)
        {
            ReportError();
        }
    }
    return N;
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Base trailer-type class to check the proper trailer concatenation
 *
 * @note Class internal to packet-metadata-test.cc
 */
class HistoryTrailerBase : public Trailer
{
  public:
    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();
    HistoryTrailerBase();
    /**
     * Checks if the header has deserialization errors
     * @returns True if no error found.
     */
    bool IsOk() const;

  protected:
    /**
     * Signal that an error has been found in deserialization.
     */
    void ReportError();

  private:
    bool m_ok; //!< True if no error is signalled.
};

TypeId
HistoryTrailerBase::GetTypeId()
{
    static TypeId tid = TypeId("ns3::HistoryTrailerBase").SetParent<Trailer>();
    return tid;
}

HistoryTrailerBase::HistoryTrailerBase()
    : m_ok(true)
{
}

bool
HistoryTrailerBase::IsOk() const
{
    return m_ok;
}

void
HistoryTrailerBase::ReportError()
{
    m_ok = false;
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Template trailer-type class to check the proper trailer concatenation
 *
 * @note Class internal to packet-metadata-test.cc
 */
template <int N>
class HistoryTrailer : public HistoryTrailerBase
{
  public:
    HistoryTrailer();

    /**
     * @brief Get the type ID.
     * @return The object TypeId.
     */
    static TypeId GetTypeId();
    TypeId GetInstanceTypeId() const override;
    void Print(std::ostream& os) const override;
    uint32_t GetSerializedSize() const override;
    void Serialize(Buffer::Iterator start) const override;
    uint32_t Deserialize(Buffer::Iterator start) override;
};

template <int N>
HistoryTrailer<N>::HistoryTrailer()
{
}

template <int N>
TypeId
HistoryTrailer<N>::GetTypeId()
{
    std::ostringstream oss;
    oss << "ns3::HistoryTrailer<" << N << ">";
    static TypeId tid =
        TypeId(oss.str()).SetParent<HistoryTrailerBase>().AddConstructor<HistoryTrailer<N>>();
    return tid;
}

template <int N>
TypeId
HistoryTrailer<N>::GetInstanceTypeId() const
{
    return GetTypeId();
}

template <int N>
void
HistoryTrailer<N>::Print(std::ostream& os) const
{
    NS_ASSERT(false);
}

template <int N>
uint32_t
HistoryTrailer<N>::GetSerializedSize() const
{
    return N;
}

template <int N>
void
HistoryTrailer<N>::Serialize(Buffer::Iterator start) const
{
    start.Prev(N);
    start.WriteU8(N, N);
}

template <int N>
uint32_t
HistoryTrailer<N>::Deserialize(Buffer::Iterator start)
{
    start.Prev(N);
    for (int i = 0; i < N; i++)
    {
        if (start.ReadU8() != N)
        {
            ReportError();
        }
    }
    return N;
}

} // namespace

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * Packet Metadata unit tests.
 */
class PacketMetadataTest : public TestCase
{
  public:
    PacketMetadataTest();
    ~PacketMetadataTest() override;
    void DoRun() override;

  private:
    /**
     * Check the packet header and trailer history.
     * Performs the check twice, once on the original packet,
     * and once on a serialized-deserialized copy of the packet.
     * @param p The packet
     * @param expected The list of expected sizes
     */
    void CheckHistory(Ptr<Packet> p, const std::list<int>& expected);

    /**
     * Does the check of the packet header and trailer history
     * @param p The packet
     * @param expected The list of expected sizes
     */
    void DoCheckHistory(Ptr<Packet> p, const std::list<int>& expected);

    /**
     * Adds an header to the packet
     * @param p The packet
     * @return The packet with the header added.
     */
    Ptr<Packet> DoAddHeader(Ptr<Packet> p);
};

PacketMetadataTest::PacketMetadataTest()
    : TestCase("Packet metadata")
{
}

PacketMetadataTest::~PacketMetadataTest()
{
}

void
PacketMetadataTest::DoCheckHistory(Ptr<Packet> p, const std::list<int>& expected)
{
    PacketMetadata::ItemIterator k = p->BeginItem();
    std::list<int> got;
    while (k.HasNext())
    {
        PacketMetadata::Item item = k.Next();
        if (item.isFragment || item.type == PacketMetadata::Item::PAYLOAD)
        {
            got.push_back(item.currentSize);
            continue;
        }
        if (item.type == PacketMetadata::Item::HEADER)
        {
            Callback<ObjectBase*> constructor = item.tid.GetConstructor();
            auto header = dynamic_cast<HistoryHeaderBase*>(constructor());
            if (header == nullptr)
            {
                goto error;
            }
            header->Deserialize(item.current);
            if (!header->IsOk())
            {
                delete header;
                goto error;
            }
            delete header;
        }
        else if (item.type == PacketMetadata::Item::TRAILER)
        {
            Callback<ObjectBase*> constructor = item.tid.GetConstructor();
            auto trailer = dynamic_cast<HistoryTrailerBase*>(constructor());
            if (trailer == nullptr)
            {
                goto error;
            }
            trailer->Deserialize(item.current);
            if (!trailer->IsOk())
            {
                delete trailer;
                goto error;
            }
            delete trailer;
        }
        got.push_back(item.currentSize);
    }

    if (expected != got)
    {
        goto error;
    }

    return;
error:
    std::ostringstream failure;
    failure << "PacketMetadata error. Got:\"";
    for (auto i = got.begin(); i != got.end(); i++)
    {
        failure << *i << ", ";
    }
    failure << "\", expected: \"";
    for (auto j = expected.begin(); j != expected.end(); j++)
    {
        failure << *j << ", ";
    }
    failure << "\"";
    NS_TEST_ASSERT_MSG_EQ(false, true, failure.str());
}

template <uint32_t N>
void
AddHeader(Ptr<Packet> p)
{
    HistoryHeader<N> header;
    p->AddHeader(header);
}

template <uint32_t N>
void
AddTrailer(Ptr<Packet> p)
{
    HistoryTrailer<N> trailer;
    p->AddTrailer(trailer);
}

template <uint32_t N>
void
RemHeader(Ptr<Packet> p)
{
    HistoryHeader<N> header;
    p->RemoveHeader(header);
}

template <uint32_t N>
void
RemTrailer(Ptr<Packet> p)
{
    HistoryTrailer<N> trailer;
    p->RemoveTrailer(trailer);
}

void
PacketMetadataTest::CheckHistory(Ptr<Packet> p, const std::list<int>& expected)
{
    DoCheckHistory(p, expected);
    uint32_t size = p->GetSerializedSize();
    uint8_t* buffer = new uint8_t[size];
    p->Serialize(buffer, size);
    Ptr<Packet> otherPacket = Create<Packet>(buffer, size, true);
    delete[] buffer;
    DoCheckHistory(otherPacket, expected);
}

Ptr<Packet>
PacketMetadataTest::DoAddHeader(Ptr<Packet> p)
{
    AddHeader<10>(p);
    return p;
}

void
PacketMetadataTest::DoRun()
{
    PacketMetadata::Enable();

    Ptr<Packet> p = Create<Packet>(0);
    Ptr<Packet> p1 = Create<Packet>(0);

    p = Create<Packet>(10);
    AddTrailer<100>(p);
    CheckHistory(p, {10, 100});

    p = Create<Packet>(10);
    AddHeader<1>(p);
    AddHeader<2>(p);
    AddHeader<3>(p);
    CheckHistory(p, {3, 2, 1, 10});
    AddHeader<5>(p);
    CheckHistory(p, {5, 3, 2, 1, 10});
    AddHeader<6>(p);
    CheckHistory(p, {6, 5, 3, 2, 1, 10});

    p = Create<Packet>(10);
    AddHeader<1>(p);
    AddHeader<2>(p);
    AddHeader<3>(p);
    RemHeader<3>(p);
    CheckHistory(p, {2, 1, 10});

    p = Create<Packet>(10);
    AddHeader<1>(p);
    AddHeader<2>(p);
    AddHeader<3>(p);
    RemHeader<3>(p);
    RemHeader<2>(p);
    CheckHistory(p, {1, 10});

    p = Create<Packet>(10);
    AddHeader<1>(p);
    AddHeader<2>(p);
    AddHeader<3>(p);
    RemHeader<3>(p);
    RemHeader<2>(p);
    RemHeader<1>(p);
    CheckHistory(p, {10});

    p = Create<Packet>(10);
    AddHeader<1>(p);
    AddHeader<2>(p);
    AddHeader<3>(p);
    p1 = p->Copy();
    RemHeader<3>(p1);
    RemHeader<2>(p1);
    RemHeader<1>(p1);
    CheckHistory(p1, {10});
    CheckHistory(p, {3, 2, 1, 10});
    AddHeader<1>(p1);
    AddHeader<2>(p1);
    CheckHistory(p1, {2, 1, 10});
    CheckHistory(p, {3, 2, 1, 10});
    AddHeader<3>(p);
    CheckHistory(p, {3, 3, 2, 1, 10});
    AddTrailer<4>(p);
    CheckHistory(p, {3, 3, 2, 1, 10, 4});
    AddTrailer<5>(p);
    CheckHistory(p, {3, 3, 2, 1, 10, 4, 5});
    RemHeader<3>(p);
    CheckHistory(p, {3, 2, 1, 10, 4, 5});
    RemTrailer<5>(p);
    CheckHistory(p, {3, 2, 1, 10, 4});
    p1 = p->Copy();
    RemTrailer<4>(p);
    CheckHistory(p, {3, 2, 1, 10});
    CheckHistory(p1, {3, 2, 1, 10, 4});
    p1->RemoveAtStart(3);
    CheckHistory(p1, {2, 1, 10, 4});
    p1->RemoveAtStart(1);
    CheckHistory(p1, {1, 1, 10, 4});
    p1->RemoveAtStart(1);
    CheckHistory(p1, {1, 10, 4});
    p1->RemoveAtEnd(4);
    CheckHistory(p1, {1, 10});
    p1->RemoveAtStart(1);
    CheckHistory(p1, {10});

    p = Create<Packet>(10);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    AddTrailer<8>(p);
    p->RemoveAtStart(8 + 10 + 8);
    CheckHistory(p, {8});

    p = Create<Packet>(10);
    AddHeader<10>(p);
    AddHeader<8>(p);
    AddTrailer<6>(p);
    AddTrailer<7>(p);
    AddTrailer<9>(p);
    p->RemoveAtStart(5);
    p->RemoveAtEnd(12);
    CheckHistory(p, {3, 10, 10, 6, 4});

    p = Create<Packet>(10);
    AddHeader<10>(p);
    AddTrailer<6>(p);
    p->RemoveAtEnd(18);
    AddTrailer<5>(p);
    AddHeader<3>(p);
    CheckHistory(p, {3, 8, 5});
    p->RemoveAtStart(12);
    CheckHistory(p, {4});
    p->RemoveAtEnd(2);
    CheckHistory(p, {2});
    AddHeader<10>(p);
    CheckHistory(p, {10, 2});
    p->RemoveAtEnd(5);
    CheckHistory(p, {7});

    Ptr<Packet> p2 = Create<Packet>(0);
    Ptr<Packet> p3 = Create<Packet>(0);

    p = Create<Packet>(40);
    AddHeader<5>(p);
    AddHeader<8>(p);
    CheckHistory(p, {8, 5, 40});
    p1 = p->CreateFragment(0, 5);
    p2 = p->CreateFragment(5, 5);
    p3 = p->CreateFragment(10, 43);
    CheckHistory(p1, {5});
    CheckHistory(p2, {3, 2});
    CheckHistory(p3, {3, 40});
    p1->AddAtEnd(p2);
    CheckHistory(p1, {8, 2});
    CheckHistory(p2, {3, 2});
    p1->AddAtEnd(p3);
    CheckHistory(p1, {8, 5, 40});
    CheckHistory(p2, {3, 2});
    CheckHistory(p3, {3, 40});
    p1 = p->CreateFragment(0, 5);
    CheckHistory(p1, {5});

    p3 = Create<Packet>(50);
    AddHeader<8>(p3);
    CheckHistory(p3, {8, 50});
    CheckHistory(p1, {5});
    p1->AddAtEnd(p3);
    CheckHistory(p1, {5, 8, 50});
    AddHeader<5>(p1);
    CheckHistory(p1, {5, 5, 8, 50});
    AddTrailer<2>(p1);
    CheckHistory(p1, {5, 5, 8, 50, 2});
    RemHeader<5>(p1);
    CheckHistory(p1, {5, 8, 50, 2});
    p1->RemoveAtEnd(60);
    CheckHistory(p1, {5});
    p1->AddAtEnd(p2);
    CheckHistory(p1, {8, 2});
    CheckHistory(p2, {3, 2});

    p3 = Create<Packet>(40);
    AddHeader<5>(p3);
    AddHeader<5>(p3);
    CheckHistory(p3, {5, 5, 40});
    p1 = p3->CreateFragment(0, 5);
    p2 = p3->CreateFragment(5, 5);
    CheckHistory(p1, {5});
    CheckHistory(p2, {5});
    p1->AddAtEnd(p2);
    CheckHistory(p1, {5, 5});

    p = Create<Packet>(0);
    CheckHistory(p, {});

    p3 = Create<Packet>(0);
    AddHeader<5>(p3);
    AddHeader<5>(p3);
    CheckHistory(p3, {5, 5});
    p1 = p3->CreateFragment(0, 4);
    p2 = p3->CreateFragment(9, 1);
    CheckHistory(p1, {4});
    CheckHistory(p2, {1});
    p1->AddAtEnd(p2);
    CheckHistory(p1, {4, 1});

    p = Create<Packet>(2000);
    CheckHistory(p, {2000});

    p = Create<Packet>();
    AddTrailer<10>(p);
    AddHeader<5>(p);
    p1 = p->CreateFragment(0, 8);
    p2 = p->CreateFragment(8, 7);
    p1->AddAtEnd(p2);
    CheckHistory(p, {5, 10});

    p = Create<Packet>();
    AddTrailer<10>(p);
    RemTrailer<10>(p);
    AddTrailer<10>(p);
    CheckHistory(p, {10});

    p = Create<Packet>();
    AddHeader<10>(p);
    RemHeader<10>(p);
    AddHeader<10>(p);
    CheckHistory(p, {10});

    p = Create<Packet>();
    AddHeader<10>(p);
    p = DoAddHeader(p);
    CheckHistory(p, {10, 10});

    p = Create<Packet>(10);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    AddTrailer<8>(p);
    p->RemoveAtStart(8 + 10 + 8);
    CheckHistory(p, {8});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    RemHeader<8>(p);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddTrailer<8>(p);
    RemTrailer<8>(p);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    p->RemoveAtStart(8);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    RemTrailer<8>(p);
    RemHeader<8>(p);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    RemHeader<8>(p);
    RemTrailer<8>(p);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    RemTrailer<8>(p);
    p->RemoveAtStart(8);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    RemHeader<8>(p);
    p->RemoveAtEnd(8);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    RemTrailer<8>(p);
    p->RemoveAtEnd(8);
    CheckHistory(p, {});

    p = Create<Packet>(0);
    AddHeader<8>(p);
    AddTrailer<8>(p);
    RemHeader<8>(p);
    p->RemoveAtStart(8);
    CheckHistory(p, {});

    p = Create<Packet>(16383);
    p = Create<Packet>(16384);

    /// @internal
    /// See \bugid{179}
    p = Create<Packet>(40);
    p2 = p->CreateFragment(5, 5);
    p3 = p->CreateFragment(10, 30);
    AddHeader<8>(p2);
    AddHeader<8>(p3);
    RemHeader<8>(p2);
    RemHeader<8>(p3);
    p2->AddAtEnd(p3);

    p = Create<Packet>(1000);
    AddHeader<10>(p);
    AddTrailer<5>(p);
    p1 = p->Copy();
    AddHeader<20>(p1);
    RemHeader<20>(p1);
    RemTrailer<5>(p1);
    NS_TEST_EXPECT_MSG_EQ(p->GetSize(), 1015, "Correct size");

    p = Create<Packet>(1510);
    AddHeader<8>(p);
    AddHeader<25>(p);
    RemHeader<25>(p);
    AddHeader<1>(p);
    p1 = p->CreateFragment(0, 1500);
    p2 = p1->Copy();
    AddHeader<24>(p2);
    NS_TEST_EXPECT_MSG_EQ(p->GetSize(), 1519, "Correct size");

    p = Create<Packet>(1000);
    AddHeader<2>(p);
    AddTrailer<3>(p);
    p1 = p->Copy();
    CheckHistory(p1, {2, 1000, 3});
    RemHeader<2>(p);
    AddHeader<1>(p);
    CheckHistory(p, {1, 1000, 3});
    CheckHistory(p1, {2, 1000, 3});

    p = Create<Packet>(200);
    AddHeader<24>(p);
    p1 = p->CreateFragment(0, 100);
    p2 = p->CreateFragment(100, 100);
    p1->AddAtEnd(p2);

    p = Create<Packet>();
    AddHeader<10>(p);
    p1 = Create<Packet>();
    AddHeader<11>(p1);
    RemHeader<11>(p1);
    p->AddAtEnd(p1);

    p = Create<Packet>(500);
    CheckHistory(p, {500});
    AddHeader<10>(p);
    CheckHistory(p, {10, 500});
    RemHeader<10>(p);
    CheckHistory(p, {500});
    p->RemoveAtEnd(10);
    CheckHistory(p, {490});

    p = Create<Packet>(500);
    CheckHistory(p, {500});
    AddTrailer<10>(p);
    CheckHistory(p, {500, 10});
    RemTrailer<10>(p);
    CheckHistory(p, {500});
    p->RemoveAtStart(10);
    CheckHistory(p, {490});

    /// @internal
    /// See \bugid{1072}
    p = Create<Packet>(500);
    AddHeader<10>(p);
    AddHeader<20>(p);
    AddHeader<5>(p);
    CheckHistory(p, {5, 20, 10, 500});
    p1 = p->CreateFragment(0, 6);
    p2 = p->CreateFragment(6, 535 - 6);
    p1->AddAtEnd(p2);

    /// @internal
    /// See \bugid{1072}
    p = Create<Packet>(reinterpret_cast<const uint8_t*>("hello world"), 11);
    AddHeader<2>(p);
    CheckHistory(p, {2, 11});
    p1 = p->CreateFragment(0, 5);
    CheckHistory(p1, {2, 3});
    p2 = p->CreateFragment(5, 8);
    CheckHistory(p2, {8});

    AddHeader<8 + 2 + 2 * 6>(p1);
    AddTrailer<4>(p1);
    CheckHistory(p1, {22, 2, 3, 4});
    AddHeader<8 + 2 + 2 * 6>(p2);
    AddTrailer<4>(p2);
    CheckHistory(p2, {22, 8, 4});

    RemTrailer<4>(p1);
    RemHeader<8 + 2 + 2 * 6>(p1);
    CheckHistory(p1, {2, 3});
    RemTrailer<4>(p2);
    RemHeader<8 + 2 + 2 * 6>(p2);
    CheckHistory(p2, {8});

    p3 = p1->Copy();
    CheckHistory(p3, {2, 3});
    p3->AddAtEnd(p2);
    CheckHistory(p3, {2, 11});

    CheckHistory(p, {2, 11});
    RemHeader<2>(p);
    CheckHistory(p, {11});
    RemHeader<2>(p3);
    CheckHistory(p3, {11});

    auto buf = new uint8_t[p3->GetSize()];
    p3->CopyData(buf, p3->GetSize());
    std::string msg = std::string(reinterpret_cast<const char*>(buf), p3->GetSize());
    delete[] buf;
    NS_TEST_EXPECT_MSG_EQ(msg,
                          std::string("hello world"),
                          "Could not find original data in received packet");
}

/**
 * @ingroup network-test
 * @ingroup tests
 *
 * @brief Packet Metadata TestSuite
 */
class PacketMetadataTestSuite : public TestSuite
{
  public:
    PacketMetadataTestSuite();
};

PacketMetadataTestSuite::PacketMetadataTestSuite()
    : TestSuite("packet-metadata", Type::UNIT)
{
    AddTestCase(new PacketMetadataTest, TestCase::Duration::QUICK);
}

static PacketMetadataTestSuite g_packetMetadataTest; //!< Static variable for test initialization

//
// tablet_impl_ssd_test.cc
// Copyright (C) 2017 4paradigm.com
// Author wangtaize 
// Date 2017-04-05
//

#include "tablet/tablet_impl.h"
#include "proto/tablet.pb.h"
#include "storage/mem_table.h"
#include "storage/ticket.h"
#include "base/kv_iterator.h"
#include "gtest/gtest.h"
#include "logging.h"
#include "timer.h"
#include "base/schema_codec.h"
#include "base/flat_array.h"
#include <boost/lexical_cast.hpp>
#include <gflags/gflags.h>
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include "log/log_writer.h"
#include "log/log_reader.h"
#include "base/file_util.h"
#include "base/strings.h"

DECLARE_string(db_root_path);
DECLARE_string(ssd_root_path);
DECLARE_string(zk_cluster);
DECLARE_string(zk_root_path);
DECLARE_int32(gc_interval);
DECLARE_int32(make_snapshot_threshold_offset);
DECLARE_int32(binlog_delete_interval);

namespace rtidb {
namespace tablet {
class MockClosure : public ::google::protobuf::Closure {

public:
    MockClosure() {}
    ~MockClosure() {}
    void Run() {}
};

using ::rtidb::api::TableStatus;

inline std::string GenRand() {
    return std::to_string(rand() % 10000000 + 1);
}

void CreateBaseTablet(::rtidb::tablet::TabletImpl& tablet,
            const ::rtidb::api::TTLType& ttl_type,
            uint64_t ttl, uint64_t start_ts,
            uint32_t tid, uint32_t pid,
            const ::rtidb::common::StorageMode& mode) {

    ::rtidb::api::CreateTableRequest crequest;
    ::rtidb::api::TableMeta* table_meta = crequest.mutable_table_meta();
    table_meta->set_name("table");
    table_meta->set_tid(tid);
    table_meta->set_pid(pid);
    table_meta->set_ttl(ttl);
    table_meta->set_seg_cnt(8);
    table_meta->set_mode(::rtidb::api::TableMode::kTableLeader);
    table_meta->set_storage_mode(mode);
    table_meta->set_key_entry_max_height(8);
    table_meta->set_ttl_type(ttl_type);
    ::rtidb::common::ColumnDesc* desc = table_meta->add_column_desc();
    desc->set_name("card");
    desc->set_type("string");
    desc->set_add_ts_idx(true);
    desc = table_meta->add_column_desc();
    desc->set_name("mcc");
    desc->set_type("string");
    desc->set_add_ts_idx(true);
    desc = table_meta->add_column_desc();
    desc->set_name("price");
    desc->set_type("int64");
    desc->set_add_ts_idx(false);
    desc = table_meta->add_column_desc();
    desc->set_name("ts1");
    desc->set_type("int64");
    desc->set_add_ts_idx(false);
    desc->set_is_ts_col(true);
    desc = table_meta->add_column_desc();
    desc->set_name("ts2");
    desc->set_type("int64");
    desc->set_add_ts_idx(false);
    desc->set_is_ts_col(true);
    desc->set_ttl(ttl);
    ::rtidb::common::ColumnKey* column_key = table_meta->add_column_key();
    column_key->set_index_name("card");
    column_key->add_ts_name("ts1");
    column_key->add_ts_name("ts2");
    column_key = table_meta->add_column_key();
    column_key->set_index_name("mcc");
    column_key->add_ts_name("ts1");
    ::rtidb::api::CreateTableResponse cresponse;
    MockClosure closure;
    tablet.CreateTable(NULL, &crequest, &cresponse,
                &closure);
    ASSERT_EQ(0, cresponse.code());

    for (int i = 0; i < 1000; i++) {
        ::rtidb::api::PutRequest request;
        request.set_tid(tid);
        request.set_pid(pid);
        ::rtidb::api::Dimension* dim = request.add_dimensions();
        dim->set_idx(0);
        dim->set_key("card" + std::to_string(i % 100));
        dim = request.add_dimensions();
        dim->set_idx(1);
        dim->set_key("mcc" + std::to_string(i));
        ::rtidb::api::TSDimension* ts = request.add_ts_dimensions();
        ts->set_idx(0);
        ts->set_ts(start_ts + i);
        ts = request.add_ts_dimensions();
        ts->set_idx(1);
        ts->set_ts(start_ts + i);
        std::string value = "value" + std::to_string(i);
        request.set_value(value);
        ::rtidb::api::PutResponse response;

        MockClosure closure;
        tablet.Put(NULL, &request, &response,
                &closure);
        ASSERT_EQ(0, response.code());
    }
}

class TabletSSDTest : public ::testing::Test {

public:
    TabletSSDTest() {}
    ~TabletSSDTest() {}
};

TEST_F(TabletSSDTest, Test_write){
    ::rtidb::tablet::TabletImpl tablet_impl;
    tablet_impl.Init();
    for (uint32_t i = 0; i < 100; i++) {
        CreateBaseTablet(tablet_impl, ::rtidb::api::TTLType::kLatestTime, 10, 1000,
            i+1, i % 10, ::rtidb::common::StorageMode::kSSD);
    }
}

}
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    srand (time(NULL));
    FLAGS_ssd_root_path="/tmp/ssd"+::rtidb::tablet::GenRand()+",/tmp/ssd" + ::rtidb::tablet::GenRand();
    return RUN_ALL_TESTS();
}


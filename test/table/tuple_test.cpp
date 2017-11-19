/**
 * tuple_test.cpp
 */

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>

#include "buffer/buffer_pool_manager.h"
#include "table/table_heap.h"
#include "table/tuple.h"
#include "vtable/virtual_table.h"
#include "gtest/gtest.h"

namespace cmudb {
TEST(TupleTest, TableHeapTest) {
  // test1: parse create sql statement
  std::string createStmt =
      "a varchar, b smallint, c bigint, d bool, e varchar(16)";
  Schema *schema = ParseCreateStatement(createStmt);
  // test2: create one tuple
  std::vector<Value> values;
  Value v(TypeId::INVALID);
  for (int i = 0; i < schema->GetColumnCount(); i++) {
    // get type
    TypeId type = schema->GetType(i);
    switch (type) {
    case TypeId::BOOLEAN:
      v = Value(type, 0);
      break;
    case TypeId::TINYINT:
      v = Value(type, (int8_t)30);
      break;
    case TypeId::SMALLINT:
    case TypeId::INTEGER:
      v = Value(type, (int32_t)200);
      break;
    case TypeId::BIGINT:
      v = Value(type, (int64_t)1000);
      break;
    case TypeId::VARCHAR:
      v = Value(type, "Hello World", 12, true);
      break;
    default:
      break;
    }
    values.emplace_back(v);
  }

  Tuple tuple(values, schema);
  std::cout << tuple.ToString(schema) << '\n';

  // create transaction
  Transaction *transaction = new Transaction(0);
  DiskManager *disk_manager = new DiskManager("test.db");
  BufferPoolManager *buffer_pool_manager =
      new BufferPoolManager(50, disk_manager);
  LockManager *lock_manager = new LockManager(true);
  LogManager *log_manager = new LogManager(disk_manager);
  TableHeap *table = new TableHeap(buffer_pool_manager, lock_manager,
                                   log_manager, transaction);

  RID rid;
  std::vector<RID> rid_v;
  for (int i = 0; i < 5000; ++i) {
    table->InsertTuple(tuple, rid, transaction);
    // std::cout << rid << '\n';
    rid_v.push_back(rid);
  }

  TableIterator itr = table->begin(transaction);
  while (itr != table->end()) {
    // std::cout << itr->ToString(schema) << std::endl;
    ++itr;
  }

  // int i = 0;
  std::random_shuffle(rid_v.begin(), rid_v.end());
  for (auto rid : rid_v) {
    // std::cout << i++ << std::endl;
    assert(table->MarkDelete(rid, transaction) == 1);
  }
  remove("test.db"); // remove db file
  remove("test.log");
  delete schema;
  delete table;
  delete buffer_pool_manager;
  delete disk_manager;
}

} // namespace cmudb

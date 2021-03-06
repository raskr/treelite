/*!
 * Copyright (c) 2020 by Contributors
 * \file test_serializer.cc
 * \author Hyunsu Cho
 * \brief C++ tests for model serializer
 */
#include <gtest/gtest.h>
#include <treelite/tree.h>
#include <treelite/frontend.h>
#include <dmlc/memory_io.h>
#include <string>
#include <memory>

namespace {

inline std::string TreeliteToBytes(treelite::Model* model) {
  std::string s;
  std::unique_ptr<dmlc::Stream> mstrm{new dmlc::MemoryStringStream(&s)};
  model->ReferenceSerialize(mstrm.get());
  mstrm.reset();
  return s;
}

inline void TestRoundTrip(treelite::Model* model) {
  auto buffer = model->GetPyBuffer();
  std::unique_ptr<treelite::Model> received_model{new treelite::Model()};
  received_model->InitFromPyBuffer(buffer);

  ASSERT_EQ(TreeliteToBytes(model), TreeliteToBytes(received_model.get()));
}

}  // anonymous namespace

namespace treelite {

TEST(PyBufferInterfaceRoundTrip, TreeStump) {
  std::unique_ptr<frontend::ModelBuilder> builder{
    new frontend::ModelBuilder(2, 1, false)
  };
  std::unique_ptr<frontend::TreeBuilder> tree{new frontend::TreeBuilder()};
  tree->CreateNode(0);
  tree->CreateNode(1);
  tree->CreateNode(2);
  tree->SetNumericalTestNode(0, 0, "<", 0.0f, true, 1, 2);
  tree->SetRootNode(0);
  tree->SetLeafNode(1, -1.0f);
  tree->SetLeafNode(2, 1.0f);
  builder->InsertTree(tree.get());

  std::unique_ptr<Model> model{new Model()};
  builder->CommitModel(model.get());
  TestRoundTrip(model.get());
}

TEST(PyBufferInterfaceRoundTrip, TreeStumpLeafVec) {
  std::unique_ptr<frontend::ModelBuilder> builder{
      new frontend::ModelBuilder(2, 2, true)
  };
  std::unique_ptr<frontend::TreeBuilder> tree{new frontend::TreeBuilder()};
  tree->CreateNode(0);
  tree->CreateNode(1);
  tree->CreateNode(2);
  tree->SetNumericalTestNode(0, 0, "<", 0.0f, true, 1, 2);
  tree->SetRootNode(0);
  tree->SetLeafVectorNode(1, {-1.0f, 1.0f});
  tree->SetLeafVectorNode(2, {1.0f, -1.0f});
  builder->InsertTree(tree.get());

  std::unique_ptr<Model> model{new Model()};
  builder->CommitModel(model.get());
  TestRoundTrip(model.get());
}

TEST(PyBufferInterfaceRoundTrip, TreeStumpCategoricalSplit) {
  std::unique_ptr<frontend::ModelBuilder> builder{
      new frontend::ModelBuilder(2, 1, false)
  };
  std::unique_ptr<frontend::TreeBuilder> tree{new frontend::TreeBuilder()};
  tree->CreateNode(0);
  tree->CreateNode(1);
  tree->CreateNode(2);
  tree->SetCategoricalTestNode(0, 0, {0, 1}, true, 1, 2);
  tree->SetRootNode(0);
  tree->SetLeafNode(1, -1.0f);
  tree->SetLeafNode(2, 1.0f);
  builder->InsertTree(tree.get());

  std::unique_ptr<Model> model{new Model()};
  builder->CommitModel(model.get());
  TestRoundTrip(model.get());
}

TEST(PyBufferInterfaceRoundTrip, TreeDepth2) {
  std::unique_ptr<frontend::ModelBuilder> builder{
      new frontend::ModelBuilder(2, 1, false)
  };
  builder->SetModelParam("pred_transform", "sigmoid");
  builder->SetModelParam("global_bias", "0.5");
  for (int tree_id = 0; tree_id < 2; ++tree_id) {
    std::unique_ptr<frontend::TreeBuilder> tree{new frontend::TreeBuilder()};
    for (int i = 0; i < 7; ++i) {
      tree->CreateNode(i);
    }
    tree->SetNumericalTestNode(0, 0, "<", 0.0f, true, 1, 2);
    tree->SetCategoricalTestNode(1, 0, {0, 1}, true, 3, 4);
    tree->SetCategoricalTestNode(2, 1, {0}, true, 5, 6);
    tree->SetRootNode(0);
    tree->SetLeafNode(3, -2.0f);
    tree->SetLeafNode(4, 1.0f);
    tree->SetLeafNode(5, -1.0f);
    tree->SetLeafNode(6, 2.0f);
    builder->InsertTree(tree.get());
  }

  std::unique_ptr<Model> model{new Model()};
  builder->CommitModel(model.get());
  TestRoundTrip(model.get());
}

TEST(PyBufferInterfaceRoundTrip, DeepFullTree) {
  const int depth = 19;

  std::unique_ptr<frontend::ModelBuilder> builder{
      new frontend::ModelBuilder(3, 1, false)
  };
  std::unique_ptr<frontend::TreeBuilder> tree{new frontend::TreeBuilder()};
  for (int level = 0; level <= depth; ++level) {
    for (int i = 0; i < (1 << level); ++i) {
      const int nid = (1 << level) - 1 + i;
      tree->CreateNode(nid);
    }
  }
  for (int level = 0; level <= depth; ++level) {
    for (int i = 0; i < (1 << level); ++i) {
      const int nid = (1 << level) - 1 + i;
      const float leaf_value = 0.5;
      if (level == depth) {
        tree->SetLeafNode(nid, leaf_value);
      } else {
        tree->SetNumericalTestNode(nid, (level % 2), "<", 0.0, true, 2 * nid + 1, 2 * nid + 2);
      }
    }
  }
  tree->SetRootNode(0);
  builder->InsertTree(tree.get());

  std::unique_ptr<Model> model{new Model()};
  builder->CommitModel(model.get());
  TestRoundTrip(model.get());
}

}  // namespace treelite

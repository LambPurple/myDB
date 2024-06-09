
#include <cstring>
#include <cstdint>

#include "util/merkletree.h"
#include "gtest/gtest.h"

// �������ֵ����͹�ϣ����
const int D_TEST_VALUES = 17;
const int LENGTH = 32;  // �����ϣ����Ϊ32�ֽ�
namespace mydb {
// ������
class MerkleTreeTest : public ::testing::Test {
protected:
    //ʵ����merkle��
    mt_t* mt;
    //����32�ֽڹ�ϣ�ַ�
    uint8_t test_values[D_TEST_VALUES][LENGTH];

    //��ʼ��
    void SetUp() override {
        mt = mt_create();
        for (uint32_t i = 0; i < D_TEST_VALUES; ++i) {
            for (uint32_t c = 0; c < LENGTH; ++c) {
                test_values[i][c] = i;
            }
        }
    }

    //�ͷſռ�
    void TearDown() override {
        mt_delete(mt);
    }

    // ����������������Ӳ���ֵ��Merkle Tree
    bool add(const uint8_t* h) {
        return mt_add(mt, h, LENGTH) == MT_SUCCESS;
    }

    // �������������ڸ���Merkle Tree
    bool update(const uint8_t* h, uint32_t offset) {
        return mt_update(mt, h, LENGTH, offset) == MT_SUCCESS;
    }
};

// ���Լ���
TEST_F(MerkleTreeTest, SimpleTree) {
    unsigned char ROOT_5_1[LENGTH] = {0x24, 0x39, 0x37, 0xFE, 0x91, 0xB8,
        0xAF, 0xCC, 0xF7, 0x79, 0x51, 0xAF, 0x4E, 0x94, 0x6C, 0x99, 0x3E, 0x21,
        0xCF, 0xE1, 0x34, 0x64, 0x4F, 0xAD, 0x15, 0xDA, 0x30, 0x2E, 0xF0, 0x93,
        0xAE, 0x68};
    unsigned char ROOT_5_2[LENGTH] = {0x8F, 0x6F, 0xA8, 0x61, 0x7E, 0x7B,
        0x9D, 0x00, 0x4C, 0xF8, 0xE7, 0x4B, 0x54, 0x25, 0xF9, 0xA5, 0xA7, 0x27,
        0x37, 0xFD, 0xE3, 0xAD, 0x35, 0x73, 0xBB, 0xD1, 0x81, 0xD8, 0xD2, 0x60,
        0x82, 0x7F};
    mt_hash_t root;
    memset(root, 0, sizeof(mt_hash_t));
    
    for (uint32_t i = 0; i < 5; ++i) {
        ASSERT_TRUE(add(test_values[i]));
        mt_get_root(mt,root);
    }
    for (uint32_t i = 0; i < 5; ++i) {
        ASSERT_TRUE(mt_verify(mt, test_values[i], LENGTH, i) == MT_SUCCESS);
    }
    ASSERT_TRUE(mt_get_root(mt, root) == MT_SUCCESS);
    ASSERT_EQ(0, memcmp(ROOT_5_1, root, LENGTH));

    ASSERT_TRUE(update(test_values[7], 0));
    ASSERT_TRUE(mt_get_root(mt, root) == MT_SUCCESS);
    ASSERT_EQ(0, memcmp(ROOT_5_2, root, LENGTH));
    // mt_print_hash(root);
}

// ������б��
TEST_F(MerkleTreeTest, SkewedTree) {
    for (uint32_t i = 0; i < 3; ++i) {
        ASSERT_TRUE(add(test_values[i]));
    }
    ASSERT_TRUE(update(test_values[3], 2));
}
}
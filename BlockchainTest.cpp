#include <gtest/gtest.h>
#include "Blockchain.h"

// Test fixture for the Blockchain class
class BlockchainTest : public ::testing::Test {
protected:
    Blockchain chain;
};

TEST_F(BlockchainTest, TestAddBlock) {
    // Create a new block and add it to the chain
    std::vector<Transaction> transactions;
    transactions.emplace_back(Transaction(Wallet("Alice", 1000000.0), Wallet("Bob", 0.0), 50.0));
    Block block(transactions, chain.getLastBlockHash());
    chain.addBlock(block);

    // Check that the block was added to the chain
    EXPECT_EQ(chain.getLength(), 2);
    EXPECT_EQ(chain.getLastBlock().getReward(), 50.0);
}

TEST_F(BlockchainTest, TestIsValid) {
    // Check that the chain is valid
    EXPECT_TRUE(chain.isValid());

    // Create an invalid block and add it to the chain
    std::vector<Transaction> transactions;
    transactions.emplace_back(Transaction(Wallet("Alice", 1000000.0), Wallet("Bob", 0.0), 50.0));
    Block block(transactions, "invalid_previous_hash");
    chain.addBlock(block);

    // Check that the chain is now invalid
    EXPECT_FALSE(chain.isValid());
}

TEST_F(BlockchainTest, TestPrintChain) {
    // Print the chain to the console
    chain.printChain();
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <string>
#include "Block.h"

TEST(Block, CalculateHash) {
    // Arrange
    std::vector<Transaction> transactions = {Transaction("Alice", "Bob", 10.0, 0.1)};
    std::string previousHash = "0000000000000000000000000000000000000000000000000000000000000000";
    Block block(transactions, previousHash);
    std::string expectedHash = "836ed8a2e92537c5b5f5b5d54e14d5c5e75b6aa3a6a128e6a1c6d193563aaae2";

    // Act
    std::string actualHash = block.calculateHash();

    // Assert
    ASSERT_EQ(actualHash, expectedHash);
}

TEST(Block, MineBlock_Successful) {
    // Arrange
    std::vector<Transaction> transactions = {Transaction("Alice", "Bob", 10.0, 0.1)};
    std::string previousHash = "0000000000000000000000000000000000000000000000000000000000000000";
    Block block(transactions, previousHash);
    int difficulty = 1;
    Wallet minerWallet("Miner");
    double expectedReward = 50.0;

    // Act
    block.mineBlock(difficulty, minerWallet);
    std::vector<Transaction> actualTransactions = block.getTransactions();
    std::string actualPreviousHash = block.getPreviousHash();
    std::string actualHash = block.getHash();
    double actualReward = block.getReward();

    // Assert
    ASSERT_EQ(actualTransactions.size(), 2);
    ASSERT_EQ(actualTransactions[0].getSender(), "Miner");
    ASSERT_EQ(actualTransactions[0].getRecipient(), "Miner");
    ASSERT_EQ(actualTransactions[0].getAmount(), expectedReward);
    ASSERT_EQ(actualTransactions[1].getSender(), "Alice");
    ASSERT_EQ(actualTransactions[1].getRecipient(), "Bob");
    ASSERT_EQ(actualTransactions[1].getAmount(), 10.0);
    ASSERT_EQ(actualPreviousHash, previousHash);
    ASSERT_EQ(actualHash.substr(0, difficulty), std::string(difficulty, '0'));
    ASSERT_EQ(actualReward, expectedReward);
}

TEST(Block, MineBlock_Failed) {
    // Arrange
    std::vector<Transaction> transactions = {Transaction("Alice", "Bob", 10.0, 0.1)};
    std::string previousHash = "0000000000000000000000000000000000000000000000000000000000000000";
    Block block(transactions, previousHash);
    int difficulty = 5;
    Wallet minerWallet("Miner");

    // Act
    block.mineBlock(difficulty, minerWallet);
    std::vector<Transaction> actualTransactions = block.getTransactions();
    std::string actualPreviousHash = block.getPreviousHash();
    std::string actualHash = block.getHash();
    double actualReward = block.getReward();

    // Assert
    ASSERT_EQ(actualTransactions.size(), 1);
    ASSERT_EQ(actualTransactions[0].getSender(), "Alice");
    ASSERT_EQ(actualTransactions[0].getRecipient(), "Bob");
    ASSERT_EQ(actualTransactions[0].getAmount(), 10.0);
    ASSERT_EQ(actualPreviousHash, previousHash);
    ASSERT_EQ(actualHash.substr(0, difficulty), std::string(difficulty, '0'));
    ASSERT_EQ(actualReward, 50.0);
}

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <string>
#include "Transaction.h"

// Test the constructor with a recipient list
TEST(TransactionTest, ConstructorWithRecipientList) {
    std::string sender = "Alice";
    std::string recipient1 = "Bob";
    std::string recipient2 = "Charlie";
    double amount = 100.0;
    double fee = 1.0;
    std::vector<std::string> recipientList = {recipient1, recipient2};

    Transaction transaction(sender, recipientList, amount, fee);

    EXPECT_EQ(transaction.getSender(), sender);
    EXPECT_EQ(transaction.getRecipientList(), recipientList);
    EXPECT_EQ(transaction.getAmount(), amount);
    EXPECT_EQ(transaction.getFee(), fee);
    EXPECT_EQ(transaction.getRecipient(), ""); // Empty recipient, since there are multiple recipients
}

// Test the constructor without a recipient list
TEST(TransactionTest, ConstructorWithoutRecipientList) {
    std::string sender = "Alice";
    std::string recipient = "Bob";
    double amount = 100.0;
    double fee = 1.0;

    Transaction transaction(sender, recipient, amount, fee);

    EXPECT_EQ(transaction.getSender(), sender);
    EXPECT_EQ(transaction.getRecipient(), recipient);
    EXPECT_EQ(transaction.getAmount(), amount);
    EXPECT_EQ(transaction.getFee(), fee);
    EXPECT_THAT(transaction.getRecipientList(), ::testing::IsEmpty()); // Empty recipient list, since there is only one recipient
}

// Test the set and get functions for the date field
TEST(TransactionTest, DateField) {
    std::string sender = "Alice";
    std::string recipient = "Bob";
    double amount = 100.0;
    double fee = 1.0;

    Transaction transaction(sender, recipient, amount, fee);

    EXPECT_EQ(transaction.getDate(), ""); // Date field should be empty initially

    std::string date = "2023-04-23 11:00:00";
    transaction.setDate(date);

    EXPECT_EQ(transaction.getDate(), date);
}

// Test the isValid() function with a valid transaction
TEST(TransactionTest, ValidTransaction) {
    std::string sender = "Alice";
    std::string recipient = "Bob";
    double amount = 100.0;
    double fee = 1.0;

    Transaction transaction(sender, recipient, amount, fee);

    EXPECT_TRUE(transaction.isValid());
}

// Test the isValid() function with an invalid transaction
TEST(TransactionTest, InvalidTransaction) {
    std::string sender = "Alice";
    std::string recipient = "Alice"; // Same sender and recipient
    double amount = 1000.0; // More than total sent by sender
    double fee = 1.0;
    std::vector<double> senderSent = {500.0, 400.0, 200.0}; // Suspicious pattern

    Transaction transaction(sender, recipient, amount, fee, senderSent);

    EXPECT_FALSE(transaction.isValid());
}

TEST(TransactionTest, SplitTransaction) {
    // Create a transaction with a recipient list and multiple sender sent amounts
    std::vector<double> senderSent = {1000.0, 2000.0, 1500.0};
    std::vectorstd::string recipientList = {"recipient1", "recipient2", "recipient3"};
    Transaction transaction("sender", "", 4500.0, 0.0, senderSent);
    transaction.setRecipientList(recipientList);

    // Split the transaction into multiple transactions
    std::vector<Transaction> splitTransactions = transaction.splitTransaction(transaction);

    // Check that the number of split transactions matches the number of recipients
    ASSERT_EQ(splitTransactions.size(), recipientList.size());

    // Check that each split transaction has the correct sender, recipient, amount, and fee
    double expectedAmountPerRecipient = transaction.getAmount() / recipientList.size();
    for (size_t i = 0; i < splitTransactions.size(); ++i) {
        ASSERT_EQ(splitTransactions[i].getSender(), transaction.getSender());
        ASSERT_EQ(splitTransactions[i].getRecipient(), recipientList[i]);
        ASSERT_EQ(splitTransactions[i].getAmount(), expectedAmountPerRecipient);
        ASSERT_EQ(splitTransactions[i].getFee(), transaction.getFee());
    }

    // Check that each split transaction has the same sender sent amounts as the original transaction
    for (size_t i = 0; i < splitTransactions.size(); ++i) {
        ASSERT_EQ(splitTransactions[i].getSenderSent(), senderSent);
    }
}

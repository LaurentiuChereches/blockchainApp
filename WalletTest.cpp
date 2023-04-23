#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <vector>
#include <string>
#include "Wallet.h"

TEST(Wallet, AddFunds)
{
    Wallet wallet("Alice", 100.0);
    wallet.addFunds(50.0, "Bank");
    EXPECT_EQ(wallet.getBalance(), 150.0);
    std::vector<Transaction> receivedTransactions = wallet.getReceivedTransactions();
    EXPECT_EQ(receivedTransactions.size(), 1);
    EXPECT_EQ(receivedTransactions[0].getAmount(), 50.0);
    EXPECT_EQ(receivedTransactions[0].getFee(), 0.05);
    EXPECT_EQ(receivedTransactions[0].getSender(), "Bank");
    EXPECT_EQ(receivedTransactions[0].getRecipient(), "Alice");
}

TEST(Wallet, SendMoneySingleRecipient)
{
    Wallet sender("Alice", 100.0);
    Wallet recipient("Bob");
    std::vector<std::string> recipients = {"Bob"};
    std::vector<Transaction> transactions = sender.sendMoney(50.0, recipients);
    EXPECT_EQ(sender.getBalance(), 49.95);
    EXPECT_EQ(transactions.size(), 1);
    EXPECT_EQ(transactions[0].getAmount(), 50.0);
    EXPECT_EQ(transactions[0].getFee(), 0.05);
    EXPECT_EQ(transactions[0].getSender(), "Alice");
    EXPECT_EQ(transactions[0].getRecipient(), "Bob");
    EXPECT_EQ(recipient.getBalance(), 50.0);
    std::vector<Transaction> receivedTransactions = recipient.getReceivedTransactions();
    EXPECT_EQ(receivedTransactions.size(), 1);
    EXPECT_EQ(receivedTransactions[0].getAmount(), 50.0);
    EXPECT_EQ(receivedTransactions[0].getFee(), 0.05);
    EXPECT_EQ(receivedTransactions[0].getSender(), "Alice");
    EXPECT_EQ(receivedTransactions[0].getRecipient(), "Bob");
}

TEST(Wallet, SendMoneyMultipleRecipients) {
    Wallet sender("Sender", 100.0);
    Wallet recipient1("Recipient1", 0.0);
    Wallet recipient2("Recipient2", 0.0);
    Wallet recipient3("Recipient3", 0.0);

    std::vector<std::string> recipients = {"Recipient1", "Recipient2", "Recipient3"};
    std::vector<Transaction> transactions = sender.sendMoney(75.0, recipients);

    // Check if transactions were created and returned correctly
    ASSERT_EQ(transactions.size(), 3);
    ASSERT_EQ(transactions[0].getSender(), "Sender");
    ASSERT_EQ(transactions[0].getRecipient(), "Recipient1");
    ASSERT_EQ(transactions[0].getAmount(), 25.0);
    ASSERT_EQ(transactions[0].getFee(), 0.05);
    ASSERT_EQ(transactions[0].getSenderSent(), std::vector<double>({25.0, 0.0, 0.0}));
    ASSERT_EQ(transactions[1].getSender(), "Sender");
    ASSERT_EQ(transactions[1].getRecipient(), "Recipient2");
    ASSERT_EQ(transactions[1].getAmount(), 25.0);
    ASSERT_EQ(transactions[1].getFee(), 0.05);
    ASSERT_EQ(transactions[1].getSenderSent(), std::vector<double>({0.0, 25.0, 0.0}));
    ASSERT_EQ(transactions[2].getSender(), "Sender");
    ASSERT_EQ(transactions[2].getRecipient(), "Recipient3");
    ASSERT_EQ(transactions[2].getAmount(), 25.0);
    ASSERT_EQ(transactions[2].getFee(), 0.05);
    ASSERT_EQ(transactions[2].getSenderSent(), std::vector<double>({0.0, 0.0, 25.0}));

    // Check if sender balance and receivedTransactions list were updated correctly
    ASSERT_EQ(sender.getBalance(), 25.0);
    ASSERT_EQ(sender.getReceivedTransactions().size(), 3);

    // Check if recipient balances and receivedTransactions lists were updated correctly
    ASSERT_EQ(recipient1.getBalance(), 25.0);
    ASSERT_EQ(recipient1.getReceivedTransactions().size(), 1);
    ASSERT_EQ(recipient2.getBalance(), 25.0);
    ASSERT_EQ(recipient2.getReceivedTransactions().size(), 1);
    ASSERT_EQ(recipient3.getBalance(), 25.0);
    ASSERT_EQ(recipient3.getReceivedTransactions().size(), 1);
}

// Test sendMoney() function with insufficient wallet balance
TEST(Wallet, SendMoneyInsufficientBalance) {
    Wallet sender("Sender", 50.0);
    Wallet recipient("Recipient", 0.0);

    std::vector<std::string> recipients = {"Recipient"};
    std::vector<Transaction> transactions = sender.sendMoney(75.0, recipients);

    // Check if transactions were not created and an error was printed
    ASSERT_EQ(transactions.size(), 0);
    ASSERT_EQ(sender.getBalance(), 50.0);
    ASSERT_EQ(recipient.getBalance(), 0.0);
}

// Test receiveMoney() function with a single transaction
TEST(Wallet, ReceiveMoneySingleTransaction) {
    // Initialize sender and receiver wallets
    Wallet sender("Alice", 1000);
    Wallet receiver("Bob", 500);

    // Send money from sender to receiver
    int amount = 500;
    std::string message = "Payment for goods";
    Transaction transaction = sender.SendMoney(receiver.GetAddress(), amount, message);

    // Ensure transaction is valid and recorded in both wallets
    ASSERT_TRUE(transaction.IsValid());
    ASSERT_EQ(sender.GetBalance(), 500);
    ASSERT_EQ(receiver.GetBalance(), 1000);
    ASSERT_EQ(sender.GetTransactionHistory().size(), 1);
    ASSERT_EQ(receiver.GetTransactionHistory().size(), 1);

    // Receive money by the receiver
    receiver.ReceiveMoney(transaction);

    // Ensure receiver's balance is updated and transaction is recorded in its history
    ASSERT_EQ(receiver.GetBalance(), 1500);
    ASSERT_EQ(receiver.GetTransactionHistory().size(), 2);
}

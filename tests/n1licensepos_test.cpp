#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/system.hpp>
#include <eosio/crypto.hpp>
#include <eosio/transaction.hpp>
#include <n1licensepos.hpp>

using namespace eosio;

class n1licensepos_tester {
public:
   n1licensepos_tester() {
      test_account = name("yourtestacc"); // Replace with your test account name
      contract_account = name("yourcontract"); // Replace with your contract's account name
   }

   // Helper function to test the 'setproduct' action
   void test_setproduct() {
      // Create a test action to call the 'setproduct' action
      action test_act{
         permission_level{test_account, "active"_n},
         contract_account, // Replace with your contract's account
         "setproduct"_n,
         std::make_tuple("productname"_n, asset(1000, symbol("EOS", 4)), "tokencontract"_n, "nftcontract"_n, 12345, 67890)
      };

      // Send the action
      test_act.send();
   }


   // Helper function to test the 'addproddata' action
   void test_addproddata(name owner, uint64_t int_ref, uint64_t id) {
      // Create a test action to call the 'addproddata' action
      action test_act{
         permission_level{test_account, "active"_n},
         contract_account, // Replace with your contract's account
         "addproddata"_n,
         std::make_tuple(owner, int_ref, id)
      };

      // Send the action
      test_act.send();
   }

   // Helper function to test the 'nft_transferin' action
   void test_nft_transferin(name from, name to, std::vector<uint64_t>& assetids, std::string memo) {
      // Create a test action to trigger the 'simpleassets::transfer' action
      action transfer_action{
         permission_level{from, "active"_n},
         "simpleassets"_n, // The contract that handles NFT transfers
         "transfer"_n,
         std::make_tuple(from, to, assetids, memo)
      };

      // Send the transfer action to simulate an NFT transfer
      transfer_action.send();

   }

   // Helper function to test the 'incomingpay' action
   void test_incomingpay(name from, name to, asset quantity, std::string memo) {
      // Create a test action to trigger the 'transfer' notification
      action transfer_action{
         permission_level{from, "active"_n},
         "eosio.token"_n, // Replace with the token contract's account (assuming it's eosio.token)
         "transfer"_n,
         std::make_tuple(from, to, quantity, memo)
      };

      // Send the transfer action to simulate the payment
      transfer_action.send();

   }

   // Helper function to test the 'delproduct' action
   void test_delproduct(name product, std::string memo) {
      // Create a test action to call the 'delproduct' action
      action test_act{
         permission_level{get_self(), "active"_n},
         contract_account, // Replace with your contract's account
         "delproduct"_n,
         std::make_tuple(product, memo)
      };

      // Send the action
      test_act.send();
   }

   // Helper function to test the 'deldata' action
   void test_deldata(name product, std::string memo) {
      // Create a test action to call the 'deldata' action
      action test_act{
         permission_level{get_self(), "active"_n},
         contract_account, // Replace with your contract's account
         "deldata"_n,
         std::make_tuple(product, memo)
      };

      // Send the action
      test_act.send();
   }


   // Helper function to test the 'del1stock' action
   void test_del1stock(uint64_t id) {
      // Create a test action to call the 'del1stock' action
      action test_act{
         permission_level{get_self(), "active"_n},
         contract_account, // Replace with your contract's account
         "del1stock"_n,
         std::make_tuple(id)
      };

      // Send the action
      test_act.send();
   }


   // Helper function to test the 'delsells' action
   void test_delsells(std::string memo) {
      // Create a test action to call the 'delsells' action
      action test_act{
         permission_level{get_self(), "active"_n},
         contract_account, // Replace with your contract's account
         "delsells"_n,
         std::make_tuple(memo)
      };

      // Send the action
      test_act.send();
   }


private:
   name test_account;
   name contract_account;
};

// Define the contract's ABI and WASM
extern "C" {
   void apply(uint64_t receiver, uint64_t code, uint64_t action) {
      if (code == receiver) {
         switch (action) {
            EOSIO_DISPATCH_HELPER(n1licensepos, (setproduct))
         }
      }
   }
}

int main() {
   n1licensepos_tester t;
   t.test_setproduct();

   // Test the 'addproddata' action
   name owner = "yourowneracc"_n; // Replace with the owner's account name
   uint64_t int_ref = 12345; // Replace with the desired INT_REF
   uint64_t id = 123; // Replace with the desired ID
   t.test_addproddata(owner, int_ref, id);


   // Test the 'nft_transferin' action by simulating an NFT transfer
   name from = "fromaccount"_n; // Replace with the 'from' account name
   name to = t.get_self(); // Assuming it's testing within the contract
   std::vector<uint64_t> assetids = {123}; // Replace with the desired asset IDs
   std::string memo = "12345"; // Replace with the desired memo
   t.test_nft_transferin(from, to, assetids, memo);


   // Test the 'incomingpay' action by simulating a payment
   name from = "fromaccount"_n; // Replace with the 'from' account name
   name to = t.get_self(); // Assuming it's testing within the contract
   asset quantity = asset(1000, symbol("EOS", 4)); // Replace with the desired payment amount
   std::string memo = "12345"; // Replace with the desired memo
   t.test_incomingpay(from, to, quantity, memo);

   // Test the 'delproduct' action
   name product = "productname"_n; // Replace with the product name you want to delete
   std::string memo = "confirm"; // This should match the memo requirement in the 'delproduct' action
   t.test_delproduct(product, memo);

    // Test the 'deldata' action
   name product = "productname"_n; // Replace with the product name you want to delete
   std::string memo = "confirm"; // This should match the memo requirement in the 'deldata' action
   t.test_deldata(product, memo);

   // Test the 'del1stock' action
   uint64_t stock_id = 123; // Replace with the stock ID you want to delete
   t.test_del1stock(stock_id);

   // Test the 'delsells' action
   std::string memo = "confirm"; // This should match the memo requirement in the 'delsells' action
   t.test_delsells(memo);

   return 0;
}

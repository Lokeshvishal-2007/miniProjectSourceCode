# Bank Transaction Management System - Features

## Overview
Complete C bank account program with role-based login, password file management, record cleanup, and transaction logging.

---

## STARTUP SEQUENCE

### 1. Record Cleanup (Automatic)
- **Function:** `cleanCorruptedRecords(FILE *fPtr)`
- Scans all 100 records in `credit.dat` at startup
- Validates each record:
  - Account number must be between 1-100
  - Last name must contain only letters (isAllLetters check)
  - First name must contain only letters (isAllLetters check)
  - Balance must be >= 0.0
- Overwrites corrupted/invalid records with blank records `{0, "", "", 0.0}`
- Prints: "Cleaned corrupted record at slot X" for each fixed record
- Prints: "Total X corrupted records cleaned" at end

### 2. Password File Creation (Automatic)
- **Function:** `createPasswordFile(void)`
- Checks if `passwords.dat` exists
- If not found, creates new `passwords.dat` with:
  - Accounts: 1, 12, 40, 55, 77
  - Default password for all: `password123`
- Uses `struct accountPassword` with fields:
  - `unsigned int acctNum`
  - `char password[20]`
- Prints: "Password file created with default passwords"

---

## LOGIN SYSTEM

### Startup Menu
```
===== BANK MANAGEMENT SYSTEM =====
1 - Admin Login
2 - User Login
3 - Exit
```

### ADMIN LOGIN
- **Username:** `admin`
- **Password:** `admin123`
- Verification: `strcmp()` for both username and password
- Attempts: 3 maximum
- Failure: "Access Denied" message + `exit(-1)`
- Success: "Access Granted" message → Admin Menu

### USER LOGIN
- **Input 1:** Account Number (validated 1-100)
- **Input 2:** Password (verified against `passwords.dat`)
- Verification process:
  - Opens `passwords.dat`
  - Reads each `accountPassword` record using `fread()`
  - Compares account number and password using `strcmp()`
  - On match: fetches account name from `credit.dat`
  - Prints: "Welcome [FirstName]!"
- Error messages:
  - "Account not found" - if account number doesn't exist
  - "Incorrect password" - if password doesn't match
  - "Incorrect account" - if invalid account input
- Attempts: 3 maximum
- Failure: "Access Denied" message + `exit(-1)`
- Success: User Menu

---

## ADMIN MENU (11 Options)

```
Enter your choice
1 - Print all accounts to accounts.txt
2 - Update account balance
3 - Add new account
4 - Delete account
5 - Search account by name
6 - Deposit
7 - Withdraw
8 - Show total balance summary
9 - View transaction log
10 - Find highest and lowest balance
11 - Logout
```

### Menu Option Details

**Option 1: Print all accounts to accounts.txt**
- **Function:** `textFile(FILE *readPtr)`
- Exports all valid accounts to `accounts.txt`
- Format: 4 columns
  - Acct (6 chars)
  - Last Name (16 chars)
  - First Name (11 chars)
  - Password (12 chars) - always "password123"
  - Balance (10 chars, 2 decimals)
- Filters: Only includes records where:
  - acctNum is 1-100
  - lastName passes isAllLetters()
  - firstName passes isAllLetters()

**Option 2: Update account balance**
- **Function:** `updateRecord(FILE *fPtr)`
- Prompts for account number (1-100)
- Shows current account details
- Accepts transaction amount (positive or negative)
- Updates balance: `client.balance += transaction`
- Saves to `credit.dat`

**Option 3: Add new account**
- **Function:** `newRecord(FILE *fPtr)`
- Prompts for account number (1-100, must be available)
- Prompts for last name (letters only)
- Prompts for first name (letters only)
- Prompts for initial balance (numeric)
- Writes to `credit.dat`

**Option 4: Delete account**
- **Function:** `deleteRecord(FILE *fPtr)`
- Prompts for account number
- Overwrites record with blank record `{0, "", "", 0.0}`
- Uses fseek and fwrite to modify `credit.dat`

**Option 5: Search account by name**
- **Function:** `searchByName(FILE *fPtr)`
- Prompts for last name to search
- Loops through all 100 records
- Uses `strcmp()` to match lastName
- Prints matching accounts or "No match found"

**Option 6: Deposit**
- **Function:** `depositAmount(FILE *fPtr, unsigned int accountNum, int isUser)`
- Admin calls: asks for account number
- Prompts for deposit amount (must be > 0)
- Adds amount to account balance
- Saves to `credit.dat`
- Calls `logTransaction()` to record in `trans_log.txt`

**Option 7: Withdraw**
- **Function:** `withdrawAmount(FILE *fPtr, unsigned int accountNum, int isUser)`
- Admin calls: asks for account number
- Prompts for withdraw amount (must be > 0)
- Checks if sufficient balance
- If insufficient: prints "Insufficient funds" and cancels
- Subtracts amount from balance
- Saves to `credit.dat`
- Calls `logTransaction()` to record in `trans_log.txt`

**Option 8: Show total balance summary**
- **Function:** `showTotalBalance(FILE *fPtr)`
- Counts all active accounts (acctNum != 0)
- Sums all balances
- Prints: "Total active accounts: X"
- Prints: "Total balance: X.XX"

**Option 9: View transaction log**
- **Function:** `viewTransactionLog(void)`
- Opens `trans_log.txt` in read mode
- Reads and prints all lines sequentially
- If file doesn't exist: "No transaction log found."

**Option 10: Find highest and lowest balance**
- **Function:** `findMinMaxBalance(FILE *fPtr)`
- Loops through all 100 records
- Tracks struct with minimum balance (skip acctNum == 0)
- Tracks struct with maximum balance (skip acctNum == 0)
- Prints full details of highest balance account:
  - Account number, last name, first name, balance
- Prints full details of lowest balance account:
  - Account number, last name, first name, balance
- If no active accounts: "No active accounts found."

**Option 11: Logout**
- Exits the loop and closes the program

---

## USER MENU (4 Options)

```
Enter your choice
1 - View my account details
2 - Deposit to my account
3 - Withdraw from my account
4 - View my transaction log
5 - Logout
```

### Menu Option Details

**Option 1: View my account details**
- **Function:** `viewAccountDetails(FILE *fPtr, unsigned int accountNum)`
- Displays only the logged-in user's account
- No account number prompt (uses userAcctNum from login)
- Shows: Acct, Last Name, First Name, Balance

**Option 2: Deposit to my account**
- **Function:** `depositAmount(FILE *fPtr, unsigned int accountNum, int isUser)`
- User deposits only to their own account
- Prompts for deposit amount (must be > 0)
- Adds to account balance
- Saves to `credit.dat`
- Records transaction in `trans_log.txt`

**Option 3: Withdraw from my account**
- **Function:** `withdrawAmount(FILE *fPtr, unsigned int accountNum, int isUser)`
- User withdraws only from their own account
- Prompts for withdraw amount (must be > 0)
- Checks sufficient balance
- If insufficient: "Insufficient funds" and cancels
- Subtracts from balance
- Saves to `credit.dat`
- Records transaction in `trans_log.txt`

**Option 4: View my transaction log**
- **Function:** `viewTransactionLog(void)`
- Displays all lines from `trans_log.txt`
- For user role, they see all transactions (admin can filter if needed)

**Option 5: Logout**
- Exits the loop and closes the program

---

## TRANSACTION LOGGING

### Log File
- **Filename:** `trans_log.txt`
- **Mode:** Append mode ("a")
- **Format:** `Acct <num> | <Type>: <amount> | New Balance: <balance>`

### Example Log Entries
```
Acct 1 | Deposit: 500.00 | New Balance: 2722.00
Acct 12 | Withdraw: 100.00 | New Balance: 900.00
Acct 40 | Deposit: 250.50 | New Balance: 1250.50
```

### Recording
- **Function:** `logTransaction(unsigned int accountNum, const char *type, double amount, double newBalance)`
- Called after every deposit or withdrawal
- Both admin and user transactions are logged
- Opens file, writes line, closes file

---

## DATA STRUCTURES

### clientData
```c
struct clientData {
    unsigned int acctNum;        // Account number (1-100)
    char lastName[15];           // Account holder last name
    char firstName[10];          // Account holder first name
    double balance;              // Account balance
};
```

### accountPassword
```c
struct accountPassword {
    unsigned int acctNum;        // Account number
    char password[20];           // Account password
};
```

---

## INPUT VALIDATION FUNCTIONS

### `int isAllLetters(const char *text)`
- Checks if string contains only alphabetic characters
- Returns 0 if NULL, empty, or contains non-letters
- Returns 1 if all characters are letters

### `int isAllDigits(const char *text)`
- Checks if string contains only numeric digits
- Returns 0 if NULL, empty, or contains non-digits
- Returns 1 if all characters are digits

### `int readValidAccountNumber(unsigned int *accountNum, const char *prompt)`
- Prompts user for account number
- Validates input as all digits
- Validates range 1-100
- Retries on invalid input
- Returns 1 on success, 0 on failure

### `int readValidAmount(double *amount, const char *prompt)`
- Prompts user for monetary amount
- Validates input as valid number using strtod()
- Retries on invalid input
- Returns 1 on success, 0 on failure

---

## FILES USED

| Filename | Type | Purpose |
|----------|------|---------|
| `credit.dat` | Binary | Stores 100 clientData records (fixed-size random access) |
| `passwords.dat` | Binary | Stores accountPassword records for user login |
| `accounts.txt` | Text | Exported report of all accounts (admin option) |
| `trans_log.txt` | Text | Transaction history log (append-only) |

---

## KEY FEATURES SUMMARY

✓ **Record Cleanup:** Automatic validation and corruption removal at startup
✓ **Password Management:** Separate password file for user authentication
✓ **Role-Based Access:** Admin vs. User with different menus and capabilities
✓ **Account-Based Login:** Users authenticate with their own account + password
✓ **Transaction Logging:** Every deposit/withdrawal is recorded with timestamp
✓ **Search & Filter:** Search accounts by last name
✓ **Summary Reports:** View active account count and total balance
✓ **Min/Max Analysis:** Find accounts with highest and lowest balances
✓ **Input Validation:** All numeric and text inputs are validated
✓ **File I/O:** Proper use of fseek, fread, fwrite for binary file access
✓ **Error Handling:** 3-attempt limits on login, insufficient funds checks

---

## COMPILATION

```bash
gcc -fdiagnostics-color=always -g trans.c -o trans.exe
```

## USAGE

```
$ ./trans.exe

===== BANK MANAGEMENT SYSTEM =====
1 - Admin Login
2 - User Login
3 - Exit
Enter choice: 1
Enter Username: admin
Enter Password: admin123
Access Granted

[Admin Menu displays...]
```

---

## PASSWORD DEFAULTS

| Account | Default Password |
|---------|------------------|
| 1 | password123 |
| 12 | password123 |
| 40 | password123 |
| 55 | password123 |
| 77 | password123 |

Admin credentials: `admin` / `admin123`

---

End of Features Documentation

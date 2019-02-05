# 臺灣大學系統程式設計 2018 年度 MP2 評測

## 使用方式

1. 第一次使用請先安裝相依套件。

    ```sh
    pip install --user coloredlogs sortedcontainers
    ```

2. 準備你的 git repository，請注意 **不要拿你寫作業的 repo 拿來評測**、**不要拿你寫作業的 repo 拿來評測**、**不要拿你寫作業的 repo 拿來評測**，爲了評測公平性，程式會自動把 repo 下面所有的修改全部復歸，請另外 git clone 一份 repo 避免你的工作遺失。

    ```sh
    mkdir my_work
    cd my_work
    git clone https://github.com/SystemProgrammingatNTU/SP18-B01902113.git
    ```

3. 修改 `github_accounts.csv`，只留下有你學號的那一行，例如

    ```
    b01902113,jerry73204
    ```

4. 執行評測，結果會跑出一個 `judge_log` 目錄，會包含你所有的評測記錄。


    ```sh
    ./judge.py github_accounts.csv my_work
    ```

5. 程式提供兩個選用的選項， `--log-level` 可以讓記錄更詳細（同時也比較難閱讀），`--keep-tmp-files` 使得評測結束不會清理測資，如此你可以進去 `judge_tmp` 觀看測資。

    ```sh
    ./judge.py --log-level DEBUG --keep-tmp-files github_accounts.csv my_work
    ```

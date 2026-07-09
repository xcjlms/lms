<template>
  <div class="user-center">
    <h2>个人中心 - 我的借阅</h2>
    <div class="tabs">
      <button :class="{ active: activeTab === 'current' }" @click="activeTab = 'current'">未归还列表</button>
      <button :class="{ active: activeTab === 'history' }" @click="activeTab = 'history'">历史记录</button>
    </div>

    <section v-if="activeTab === 'current'">
      <div class="summary">未归还图书：{{ currentBorrows.length }} 条</div>
      <table class="borrow-table">
        <thead>
          <tr>
            <th>图书名</th>
            <th>借出日期</th>
            <th>应还日期</th>
            <th>逾期天数</th>
            <th>产生罚款</th>
            <th>操作</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="item in currentBorrows" :key="item.id">
            <td>{{ item.title }}</td>
            <td>{{ item.borrowDate }}</td>
            <td>{{ item.dueDate }}</td>
            <td>{{ item.overdueDays }}</td>
            <td>{{ item.fine ? '¥' + item.fine : '无' }}</td>
            <td>
              <button @click="returnBook(item)">去归还</button>
              <button @click="payFine(item)" :disabled="!item.fine">缴纳罚款</button>
            </td>
          </tr>
        </tbody>
      </table>
    </section>

    <section v-else>
      <div class="history-filter">
        <label>时间范围：
          <input type="date" v-model="historyStart" />
          -
          <input type="date" v-model="historyEnd" />
        </label>
        <button @click="clearHistoryFilter">重置</button>
      </div>
      <table class="borrow-table">
        <thead>
          <tr>
            <th>图书名</th>
            <th>借出日期</th>
            <th>归还时间</th>
            <th>应还日期</th>
          </tr>
        </thead>
        <tbody>
          <tr v-for="item in filteredHistory" :key="item.id">
            <td>{{ item.title }}</td>
            <td>{{ item.borrowDate }}</td>
            <td>{{ item.returnDate }}</td>
            <td>{{ item.dueDate }}</td>
          </tr>
          <tr v-if="filteredHistory.length === 0">
            <td colspan="4">暂无符合条件的记录</td>
          </tr>
        </tbody>
      </table>
    </section>
  </div>
</template>

<script setup>
import { ref, computed } from 'vue'

const activeTab = ref('current')
const historyStart = ref('')
const historyEnd = ref('')

const currentBorrows = ref([
  { id: 1, title: '《算法导论》', borrowDate: '2026-06-01', dueDate: '2026-06-15', overdueDays: 0, fine: 0 },
  { id: 2, title: '《Vue.js 实战》', borrowDate: '2026-05-10', dueDate: '2026-05-30', overdueDays: 5, fine: 25 },
])

const historyRecords = ref([
  { id: 1, title: '《深入理解计算机系统》', borrowDate: '2026-04-01', returnDate: '2026-04-20', dueDate: '2026-04-18' },
  { id: 2, title: '《JavaScript 权威指南》', borrowDate: '2026-03-05', returnDate: '2026-03-12', dueDate: '2026-03-12' },
])

const filteredHistory = computed(() => {
  if (!historyStart.value || !historyEnd.value) {
    return historyRecords.value
  }

  const start = new Date(historyStart.value)
  const end = new Date(historyEnd.value)

  return historyRecords.value.filter(item => {
    const returnDate = new Date(item.returnDate)
    return returnDate >= start && returnDate <= end
  })
})

function returnBook(item) {
  console.log('去归还', item)
}

function payFine(item) {
  console.log('缴纳罚款', item)
}

function clearHistoryFilter() {
  historyStart.value = ''
  historyEnd.value = ''
}
</script>

<style scoped>
.user-center {
  padding: 16px;
  background: #fff;
}

.tabs {
  margin-bottom: 16px;
}

.tabs button {
  margin-right: 8px;
  padding: 8px 14px;
  border: 1px solid #dcdfe6;
  background: #fff;
  cursor: pointer;
  border-radius: 4px;
}

.tabs button.active {
  border-color: #409eff;
  color: #409eff;
}

.summary {
  margin-bottom: 12px;
}

.borrow-table {
  width: 100%;
  border-collapse: collapse;
}

.borrow-table th,
.borrow-table td {
  border: 1px solid #ebeef5;
  padding: 10px;
  text-align: left;
}

.borrow-table th {
  background: #f5f7fa;
}

.history-filter {
  margin-bottom: 12px;
  display: flex;
  align-items: center;
  gap: 12px;
}

.history-filter input {
  padding: 4px 8px;
  border: 1px solid #dcdfe6;
  border-radius: 4px;
}

button {
  border: 1px solid #dcdfe6;
  background: #fff;
  padding: 6px 12px;
  border-radius: 4px;
  cursor: pointer;
}

button:disabled {
  color: #909399;
  border-color: #ebeef5;
  cursor: not-allowed;
}
</style>

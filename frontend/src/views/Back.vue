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
          <tr v-for="item in currentBorrows" :key="item.record_id">
            <td>{{ item.book_title }}</td>
            <td>{{ item.borrow_date }}</td>
            <td>{{ item.due_date }}</td>
            <td>{{ item.overdue_days || '-' }}</td>
            <td>{{ item.fine ? '¥' + item.fine : '无' }}</td>
            <td>
              <button @click="returnBook(item)">去归还</button>
              <button @click="payFine(item)">缴纳罚款</button>
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
          <tr v-for="item in filteredHistory" :key="item.record_id">
            <td>{{ item.book_title }}</td>
            <td>{{ item.borrow_date }}</td>
            <td>{{ item.return_date }}</td>
            <td>{{ item.due_date }}</td>
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
import { ref, computed, onMounted } from 'vue'
import * as api from '../api/index.js'

const raw = localStorage.getItem('lms_user')
const userInfo = raw ? JSON.parse(raw) : {}
const uid = userInfo.user_id

const activeTab = ref('current')
const historyStart = ref('')
const historyEnd = ref('')
const message = ref('')

const allRecords = ref([])

const currentBorrows = computed(() =>
  allRecords.value.filter(r => r.status === 'BORROWED' || r.status === 'OVERDUE')
)

const historyRecords = computed(() =>
  allRecords.value.filter(r => r.status === 'RETURNED')
)

const filteredHistory = computed(() => {
  if (!historyStart.value || !historyEnd.value) return historyRecords.value
  const start = new Date(historyStart.value)
  const end = new Date(historyEnd.value)
  return historyRecords.value.filter(item => {
    const d = new Date(item.return_date || item.due_date)
    return d >= start && d <= end
  })
})

async function fetchRecords() {
  try {
    const res = await api.getBorrowHistory(uid)
    allRecords.value = res.data?.records || []
  } catch (e) {
    message.value = '加载借阅记录失败'
  }
}

async function returnBook(item) {
  try {
    await api.returnBook(uid, item.book_id)
    message.value = `《${item.book_title}》归还成功！`
    await fetchRecords()
  } catch (e) {
    message.value = '归还失败：' + e.message
  }
}

async function payFine(item) {
  // Find fine_id for this record
  try {
    const fineRes = await api.getUserFines(uid)
    const fines = fineRes.data?.fines || []
    const fine = fines.find(f => f.record_id === item.record_id && f.status === 'UNPAID')
    if (!fine) {
      message.value = '未找到未支付的罚款记录'
      return
    }
    await api.payFine(fine.fine_id)
    message.value = '罚款缴纳成功！'
    await fetchRecords()
  } catch (e) {
    message.value = '支付失败：' + e.message
  }
}

function clearHistoryFilter() {
  historyStart.value = ''
  historyEnd.value = ''
}

onMounted(fetchRecords)
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

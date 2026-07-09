<template>
  <div class="fine-page">
    <h2>我的罚款</h2>
    <div class="toolbar">
      <button :disabled="!hasSelection" @click="paySelected">批量支付</button>
      <span class="summary">已选 {{ selectedFineIds.length }} 条 / 总计 {{ selectedCountAmount }} 元</span>
    </div>
    <table class="fine-table">
      <thead>
        <tr>
          <th><input type="checkbox" :checked="allSelected" @change="toggleSelectAll($event)" /></th>
          <th>借阅记录</th>
          <th>逾期天数</th>
          <th>罚款金额</th>
          <th>状态</th>
          <th>操作</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="fine in fines" :key="fine.id">
          <td>
            <input type="checkbox" :value="fine.id" v-model="selectedFineIds" :disabled="fine.status === '已结清'" />
          </td>
          <td>
            <div class="record-title">{{ fine.bookTitle }}</div>
            <div class="record-info">借阅ID：{{ fine.borrowId }} / 借阅人：{{ fine.borrowerName }}</div>
            <div class="record-info">借出：{{ fine.borrowDate }} - 归还：{{ fine.returnDate }}</div>
          </td>
          <td>{{ fine.overdueDays }} 天</td>
          <td>{{ fine.amount.toFixed(2) }} 元</td>
          <td>
            <span :class="['status', fine.status === '已结清' ? 'paid' : 'unpaid']">{{ fine.status }}</span>
          </td>
          <td>
            <button @click="payFine(fine.id)" :disabled="fine.status === '已结清'">支付</button>
          </td>
        </tr>
        <tr v-if="fines.length === 0">
          <td colspan="6">暂无罚款记录</td>
        </tr>
      </tbody>
    </table>
  </div>
</template>

<script setup>
import { ref, computed, onMounted } from 'vue'
import * as api from '../api/index.js'

const raw = localStorage.getItem('lms_user')
const userInfo = raw ? JSON.parse(raw) : {}
const uid = userInfo.user_id

const selectedFineIds = ref([])
const fines = ref([])

// Fetch fines and merge with borrow records for book titles
async function fetchFines() {
  try {
    const [fineRes, histRes] = await Promise.all([
      api.getUserFines(uid),
      api.getBorrowHistory(uid),
    ])
    const rawFines = fineRes.data?.fines || []
    const records = histRes.data?.records || []

    fines.value = rawFines.map(f => {
      const record = records.find(r => r.record_id === f.record_id) || {}
      return {
        id: f.fine_id,
        record_id: f.record_id,
        bookTitle: record.book_title || `记录 #${f.record_id}`,
        borrowerName: userInfo.username,
        borrowDate: record.borrow_date || '',
        returnDate: record.return_date || f.create_time,
        overdueDays: record.overdue_days || '-',
        amount: f.amount,
        status: f.status === 'PAID' ? '已结清' : '未支付',
      }
    })
  } catch (e) {
    console.error('Failed to load fines:', e)
  }
}

const hasSelection = computed(() => selectedFineIds.value.length > 0)

const allSelected = computed({
  get() {
    const unpaidIds = fines.value.filter(f => f.status !== '已结清').map(f => f.id)
    return unpaidIds.length > 0 && unpaidIds.every(id => selectedFineIds.value.includes(id))
  },
  set(value) {
    if (value) {
      selectedFineIds.value = fines.value.filter(f => f.status !== '已结清').map(f => f.id)
    } else {
      selectedFineIds.value = []
    }
  },
})

const selectedCountAmount = computed(() =>
  fines.value
    .filter(f => selectedFineIds.value.includes(f.id))
    .reduce((sum, f) => sum + f.amount, 0)
    .toFixed(2)
)

async function payFine(id) {
  try {
    await api.payFine(id)
    await fetchFines()
    selectedFineIds.value = selectedFineIds.value.filter(fid => fid !== id)
  } catch (e) {
    console.error('Payment failed:', e)
  }
}

async function paySelected() {
  const ids = [...selectedFineIds.value]
  for (const id of ids) {
    try { await api.payFine(id) } catch (e) { console.error(e) }
  }
  await fetchFines()
  selectedFineIds.value = []
}

function toggleSelectAll(event) {
  allSelected.value = event.target.checked
}

onMounted(fetchFines)
</script>

<style scoped>
.fine-page {
  padding: 20px;
  background: #fff;
}
.fine-page h2 {
  margin-bottom: 16px;
}
.toolbar {
  margin-bottom: 12px;
  display: flex;
  align-items: center;
  gap: 12px;
}
.toolbar button {
  padding: 6px 14px;
  cursor: pointer;
}
.toolbar button:disabled {
  opacity: 0.5;
  cursor: not-allowed;
}
.summary {
  color: #666;
}
.fine-table {
  width: 100%;
  border-collapse: collapse;
}
.fine-table th,
.fine-table td {
  border: 1px solid #e8e8e8;
  padding: 10px;
  text-align: left;
  vertical-align: top;
}
.fine-table th {
  background: #fafafa;
}
.record-title {
  font-weight: 600;
  margin-bottom: 4px;
}
.record-info {
  color: #666;
  font-size: 13px;
  line-height: 1.4;
}
.status {
  padding: 2px 8px;
  border-radius: 4px;
  font-size: 12px;
}
.status.paid {
  background: #e6f7ff;
  color: #1890ff;
  border: 1px solid #91d5ff;
}
.status.unpaid {
  background: #fff1f0;
  color: #cf1322;
  border: 1px solid #ffa39e;
}
.fine-table button {
  padding: 4px 10px;
}
.fine-table button:disabled {
  opacity: 0.6;
  cursor: not-allowed;
}
</style>

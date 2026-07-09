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

<script>
export default {
  name: 'Fine',
  data() {
    return {
      selectedFineIds: [],
      fines: [
        {
          id: 1,
          borrowId: 'BR-1001',
          bookTitle: '《算法导论》',
          borrowerName: '张三',
          borrowDate: '2026-05-01',
          returnDate: '2026-05-21',
          overdueDays: 5,
          amount: 25.0,
          status: '未支付',
        },
        {
          id: 2,
          borrowId: 'BR-1005',
          bookTitle: '《JavaScript高级程序设计》',
          borrowerName: '李四',
          borrowDate: '2026-04-12',
          returnDate: '2026-05-10',
          overdueDays: 3,
          amount: 15.0,
          status: '未支付',
        },
        {
          id: 3,
          borrowId: 'BR-1010',
          bookTitle: '《机器学习》',
          borrowerName: '王五',
          borrowDate: '2026-02-20',
          returnDate: '2026-03-05',
          overdueDays: 0,
          amount: 0.0,
          status: '已结清',
        },
      ],
    };
  },
  computed: {
    hasSelection() {
      return this.selectedFineIds.length > 0;
    },
    allSelected: {
      get() {
        const unpaidIds = this.fines.filter((f) => f.status !== '已结清').map((f) => f.id);
        return unpaidIds.length > 0 && unpaidIds.every((id) => this.selectedFineIds.includes(id));
      },
      set(value) {
        if (value) {
          this.selectedFineIds = this.fines.filter((f) => f.status !== '已结清').map((f) => f.id);
        } else {
          this.selectedFineIds = [];
        }
      },
    },
    selectedCountAmount() {
      return this.fines
        .filter((f) => this.selectedFineIds.includes(f.id))
        .reduce((sum, f) => sum + f.amount, 0)
        .toFixed(2);
    },
  },
  methods: {
    payFine(id) {
      const fine = this.fines.find((item) => item.id === id);
      if (!fine || fine.status === '已结清') return;
      fine.status = '已结清';
      this.selectedFineIds = this.selectedFineIds.filter((item) => item !== id);
    },
    paySelected() {
      this.fines.forEach((fine) => {
        if (this.selectedFineIds.includes(fine.id) && fine.status !== '已结清') {
          fine.status = '已结清';
        }
      });
      this.selectedFineIds = [];
    },
    toggleSelectAll(event) {
      this.allSelected = event.target.checked;
    },
  },
};
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

<template>
  <div class="borrow-page">
    <div class="detail-card">
      <div class="cover-panel">
        <img :src="book.cover" alt="图书封面" class="book-cover" />
      </div>
      <div class="info-panel">
        <div class="book-title">{{ book.title }}</div>
        <div class="book-meta">
          <p>作者：{{ book.author }}</p>
          <p>ISBN：{{ book.isbn }}</p>
          <p>出版社：{{ book.publisher }}</p>
          <p>出版年份：{{ book.year }}</p>
          <p>分类：{{ book.category }}</p>
          <p>书架位置：{{ book.location }}</p>
        </div>
        <div class="stock-info">
          <div>总库存：{{ book.totalStock }}</div>
          <div>可借数量：{{ availableStock }}</div>
        </div>
        <div class="rule-tip">最长借阅天数：{{ book.maxDays }}天</div>
        <button
          class="borrow-button"
          :class="{ disabled: availableStock <= 0 }"
          :disabled="availableStock <= 0"
          @click="openBorrowDialog"
        >
          借阅此书
        </button>
      </div>
    </div>

    <div class="tab-panel">
      <div class="tab-header">
        <button
          v-for="tab in tabs"
          :key="tab.key"
          :class="['tab-item', { active: activeTab === tab.key }]"
          @click="activeTab = tab.key"
        >
          {{ tab.label }}
        </button>
      </div>
      <div class="tab-content">
        <div v-if="activeTab === 'intro'" class="tab-section">
          <h3>图书简介</h3>
          <p>{{ book.description }}</p>
        </div>
        <div v-else-if="activeTab === 'records'" class="tab-section">
          <h3>借阅记录</h3>
          <ul class="record-list">
            <li v-for="record in borrowRecords" :key="record.id">
              <div>{{ record.reader }} · {{ record.duration }}天 · {{ record.date }}</div>
              <div>状态：{{ record.status }}</div>
            </li>
          </ul>
          <div v-if="borrowRecords.length === 0" class="empty-state">暂无借阅记录</div>
        </div>
        <div v-else-if="activeTab === 'comments'" class="tab-section">
          <h3>读者评论反馈</h3>
          <ul class="comment-list">
            <li v-for="comment in comments" :key="comment.id">
              <div class="comment-author">{{ comment.user }}</div>
              <div class="comment-text">{{ comment.content }}</div>
            </li>
          </ul>
          <div v-if="comments.length === 0" class="empty-state">暂无评论</div>
        </div>
      </div>
    </div>

    <div v-if="showDialog" class="dialog-mask" @click.self="closeDialog">
      <div class="dialog-box">
        <h3>确认借阅</h3>
        <p>请确认是否借阅《{{ book.title }}》</p>
        <div class="duration-select">
          <label>借阅时长：</label>
          <select v-model.number="borrowDuration">
            <option v-for="day in durationOptions" :key="day" :value="day">{{ day }}天</option>
          </select>
        </div>
        <div class="dialog-actions">
          <button class="cancel-button" @click="closeDialog">取消</button>
          <button class="confirm-button" @click="confirmBorrow">确认借阅</button>
        </div>
      </div>
    </div>
  </div>
</template>

<script setup>
import { reactive, ref, computed } from 'vue'

const book = reactive({
  title: 'Vue 项目实战',
  author: '张三',
  isbn: '978-7-111-12345-6',
  publisher: '某某出版社',
  year: 2023,
  category: '计算机/前端',
  location: 'A区-03架',
  totalStock: 8,
  available: 5,
  maxDays: 30,
  cover: 'https://via.placeholder.com/240x320?text=书籍封面',
  description: '本书详细介绍了 Vue 3 项目构建、组件设计、状态管理、路由、打包部署等实战内容，适合前端开发者快速上手。',
})

const borrowRecords = ref([
  { id: 1, reader: '李四', duration: 14, date: '2026-06-28', status: '已归还' },
  { id: 2, reader: '王五', duration: 21, date: '2026-06-10', status: '逾期中' },
])

const comments = ref([
  { id: 1, user: '小明', content: '内容实用，示例清晰，适合入门。' },
  { id: 2, user: '小红', content: '章节安排合理，代码示例很有帮助。' },
])

const tabs = [
  { key: 'intro', label: '图书简介' },
  { key: 'records', label: '借阅记录' },
  { key: 'comments', label: '读者评论反馈' },
]

const activeTab = ref('intro')
const showDialog = ref(false)
const borrowDuration = ref(14)
const durationOptions = [7, 14, 21, 30]

const availableStock = computed(() => book.available)

function openBorrowDialog() {
  if (availableStock.value <= 0) return
  borrowDuration.value = 14
  showDialog.value = true
}

function closeDialog() {
  showDialog.value = false
}

function confirmBorrow() {
  if (book.available <= 0) return
  book.available -= 1
  borrowRecords.value.unshift({
    id: Date.now(),
    reader: '当前用户',
    duration: borrowDuration.value,
    date: new Date().toISOString().slice(0, 10),
    status: '借阅中',
  })
  closeDialog()
}
</script>

<style scoped>
.borrow-page {
  max-width: 1000px;
  margin: 0 auto;
  padding: 24px;
  color: #333;
}
.detail-card {
  display: flex;
  gap: 24px;
  margin-bottom: 24px;
  background: #fff;
  padding: 20px;
  border-radius: 12px;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.05);
}
.cover-panel {
  width: 280px;
  flex-shrink: 0;
}
.book-cover {
  width: 100%;
  border-radius: 12px;
  object-fit: cover;
}
.info-panel {
  flex: 1;
  display: flex;
  flex-direction: column;
  justify-content: space-between;
}
.book-title {
  font-size: 28px;
  font-weight: 700;
  margin-bottom: 16px;
}
.book-meta p {
  margin: 6px 0;
  color: #555;
}
.stock-info {
  margin: 16px 0;
  font-weight: 500;
}
.rule-tip {
  color: #666;
  margin-bottom: 20px;
}
.borrow-button {
  width: 160px;
  padding: 12px 0;
  border: none;
  border-radius: 6px;
  color: #fff;
  background: #409eff;
  cursor: pointer;
  font-size: 16px;
}
.borrow-button.disabled {
  background: #c0c4cc;
  cursor: not-allowed;
}
.tab-panel {
  background: #fff;
  border-radius: 12px;
  box-shadow: 0 8px 24px rgba(0, 0, 0, 0.05);
}
.tab-header {
  display: flex;
  border-bottom: 1px solid #f0f0f0;
}
.tab-item {
  padding: 16px 24px;
  cursor: pointer;
  background: transparent;
  border: none;
  font-size: 16px;
  color: #666;
}
.tab-item.active {
  color: #303133;
  border-bottom: 2px solid #409eff;
}
.tab-content {
  padding: 24px;
}
.tab-section h3 {
  margin-bottom: 12px;
}
.record-list,
.comment-list {
  list-style: none;
  padding: 0;
  margin: 0;
}
.record-list li,
.comment-list li {
  padding: 14px 0;
  border-bottom: 1px solid #f0f0f0;
}
.comment-author {
  font-weight: 600;
  margin-bottom: 6px;
}
.empty-state {
  color: #999;
  padding: 20px 0;
}
.dialog-mask {
  position: fixed;
  inset: 0;
  background: rgba(0, 0, 0, 0.35);
  display: flex;
  justify-content: center;
  align-items: center;
  z-index: 1000;
}
.dialog-box {
  width: 420px;
  background: #fff;
  border-radius: 12px;
  padding: 24px;
  box-shadow: 0 16px 40px rgba(0, 0, 0, 0.15);
}
.dialog-box h3 {
  margin-bottom: 16px;
}
.duration-select {
  display: flex;
  align-items: center;
  margin: 20px 0;
}
.duration-select label {
  margin-right: 12px;
  color: #555;
}
.duration-select select {
  flex: 1;
  padding: 8px 10px;
  border: 1px solid #dcdfe6;
  border-radius: 6px;
  background: #fff;
}
.dialog-actions {
  display: flex;
  justify-content: flex-end;
  gap: 12px;
}
.cancel-button,
.confirm-button {
  padding: 10px 18px;
  border: none;
  border-radius: 6px;
  cursor: pointer;
}
.cancel-button {
  background: #f5f7fa;
  color: #606266;
}
.confirm-button {
  background: #409eff;
  color: #fff;
}
</style>

<template>
  <div class="borrow-page">
    <div class="search-bar">
      <input v-model="keyword" placeholder="输入书名或作者搜索..." @keyup.enter="searchBooks" />
      <button @click="searchBooks">搜索</button>
    </div>
    <div v-if="message" :class="['message', message.includes('失败') ? 'error' : '']">{{ message }}</div>

    <div v-if="books.length > 0" class="book-list">
      <article
        v-for="b in books"
        :key="b.book_id"
        class="book-card"
        :class="{ selected: selectedBook?.book_id === b.book_id }"
        @click="selectBook(b)"
      >
        <div class="book-card-title">{{ b.title }}</div>
        <div class="book-card-meta">{{ b.author }} · 可借 {{ b.available_stock }}/{{ b.total_stock }}</div>
      </article>
    </div>
    <div v-if="books.length === 0 && !keyword" class="empty-state">输入关键词搜索图书</div>
    <div v-if="books.length === 0 && keyword" class="empty-state">未找到匹配的图书</div>

    <div v-if="selectedBook" class="detail-card">
      <div class="cover-panel">
        <div class="cover-placeholder">{{ selectedBook.title.charAt(0) }}</div>
      </div>
      <div class="info-panel">
        <div class="book-title">{{ selectedBook.title }}</div>
        <div class="book-meta">
          <p>作者：{{ selectedBook.author }}</p>
          <p>ISBN：{{ selectedBook.isbn }}</p>
          <p>出版社：{{ selectedBook.publisher }}</p>
          <p>书架位置：{{ selectedBook.location }}</p>
        </div>
        <div class="stock-info">
          <div>总库存：{{ selectedBook.total_stock }}</div>
          <div>可借数量：{{ selectedBook.available_stock }}</div>
        </div>
        <div class="rule-tip">最长借阅天数：30 天</div>
        <button
          class="borrow-button"
          :class="{ disabled: selectedBook.available_stock <= 0 }"
          :disabled="selectedBook.available_stock <= 0"
          @click="openBorrowDialog"
        >
          借阅此书
        </button>
      </div>
    </div>

    <div v-if="showDialog" class="dialog-mask" @click.self="closeDialog">
      <div class="dialog-box">
        <h3>确认借阅</h3>
        <p>请确认是否借阅《{{ selectedBook?.title }}》</p>
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
import { ref, onMounted } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import * as api from '../api/index.js'

const route = useRoute()
const router = useRouter()

const keyword = ref('')
const books = ref([])
const selectedBook = ref(null)
const showDialog = ref(false)
const borrowDuration = ref(14)
const durationOptions = [7, 14, 21, 30]
const message = ref('')

const raw = localStorage.getItem('lms_user')
const userInfo = raw ? JSON.parse(raw) : {}

async function searchBooks(category) {
  try {
    // 从分类入口进入时用 category 过滤；用户手动搜索时仅按关键词
    const cat = category || ''
    const res = await api.getBooks(cat ? '' : keyword.value, cat)
    books.value = res.data?.books || []
    selectedBook.value = null
    message.value = ''
  } catch (e) {
    message.value = '搜索失败：' + e.message
  }
}

function selectBook(b) {
  selectedBook.value = b
}

function openBorrowDialog() {
  if (!selectedBook.value || selectedBook.value.available_stock <= 0) return
  showDialog.value = true
  borrowDuration.value = 14
}

function closeDialog() {
  showDialog.value = false
}

async function confirmBorrow() {
  if (!selectedBook.value || !userInfo.user_id) {
    message.value = '请先登录'
    return
  }
  try {
    await api.borrowBook(userInfo.user_id, selectedBook.value.book_id, borrowDuration.value)
    message.value = `《${selectedBook.value.title}》借阅成功！`
    showDialog.value = false
    // Refresh book list
    await searchBooks()
  } catch (e) {
    message.value = '借阅失败：' + e.message
  }
}

onMounted(() => {
  const cat = route.query.category
  if (cat) {
    searchBooks(cat)
  } else {
    searchBooks()
  }
})
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
/* New styles for search & book list */
.search-bar { display: flex; gap: 12px; margin-bottom: 20px; }
.search-bar input { flex: 1; padding: 10px 14px; border: 1px solid #dcdfe6; border-radius: 8px; font-size: 15px; }
.search-bar button { padding: 10px 20px; border: none; border-radius: 8px; background: #409eff; color: #fff; cursor: pointer; }
.book-list { display: flex; flex-wrap: wrap; gap: 12px; margin-bottom: 20px; }
.book-card { padding: 14px 18px; border: 1px solid #e4e7ed; border-radius: 10px; cursor: pointer; background: #fafbfc; min-width: 200px; flex: 1 1 200px; }
.book-card.selected { border-color: #409eff; background: #ecf5ff; }
.book-card-title { font-weight: 600; margin-bottom: 4px; }
.book-card-meta { color: #888; font-size: 13px; }
.cover-placeholder { width: 240px; height: 320px; background: linear-gradient(135deg, #819cff, #aac9ff); border-radius: 12px; display: grid; place-items: center; font-size: 80px; color: #fff; }
.message { padding: 12px 16px; border-radius: 8px; margin-bottom: 16px; background: #f0f9eb; color: #67c23a; }
.message.error { background: #fef0f0; color: #f56c6c; }
.confirm-button {
  background: #409eff;
  color: #fff;
}
</style>

<template>
  <div class="book-manage">
    <h2>图书管理</h2>
    <div class="toolbar">
      <div class="search">
        <input v-model="keyword" placeholder="搜索书名/作者" @keyup.enter="fetchBooks" />
        <button @click="fetchBooks">搜索</button>
      </div>
      <button class="primary" @click="openAddDialog">添加图书</button>
    </div>

    <table class="book-table">
      <thead>
        <tr>
          <th>ID</th>
          <th>ISBN</th>
          <th>书名</th>
          <th>作者</th>
          <th>出版社</th>
          <th>总库存</th>
          <th>可借</th>
          <th>位置</th>
          <th>状态</th>
          <th>操作</th>
        </tr>
      </thead>
      <tbody>
        <tr v-for="b in books" :key="b.book_id">
          <td>{{ b.book_id }}</td>
          <td>{{ b.isbn }}</td>
          <td>{{ b.title }}</td>
          <td>{{ b.author }}</td>
          <td>{{ b.publisher }}</td>
          <td>{{ b.total_stock }}</td>
          <td>{{ b.available_stock }}</td>
          <td>{{ b.location }}</td>
          <td>{{ b.status }}</td>
          <td>
            <button @click="openEditDialog(b)">编辑</button>
            <button @click="deleteBook(b.book_id)">删除</button>
            <button @click="adjustStock(b.book_id, 1)">+1</button>
            <button @click="adjustStock(b.book_id, -1)">-1</button>
          </td>
        </tr>
        <tr v-if="books.length === 0"><td colspan="10">暂无图书</td></tr>
      </tbody>
    </table>

    <!-- 添加/编辑对话框 -->
    <div v-if="dialogVisible" class="dialog-mask" @click.self="dialogVisible = false">
      <div class="dialog-box">
        <h3>{{ isEdit ? '编辑图书' : '添加图书' }}</h3>
        <form @submit.prevent="submitBook">
          <div class="form-group">
            <label>ISBN *</label>
            <input v-model="form.isbn" required />
          </div>
          <div class="form-group">
            <label>书名 *</label>
            <input v-model="form.title" required />
          </div>
          <div class="form-group">
            <label>作者 *</label>
            <input v-model="form.author" required />
          </div>
          <div class="form-group">
            <label>出版社</label>
            <input v-model="form.publisher" />
          </div>
          <div class="form-group">
            <label>分类</label>
            <input v-model="form.category_id" />
          </div>
          <div class="form-group">
            <label>总库存 *</label>
            <input v-model.number="form.total_stock" type="number" min="0" required />
          </div>
          <div class="form-group">
            <label>位置</label>
            <input v-model="form.location" />
          </div>
          <div class="dialog-actions">
            <button type="button" @click="dialogVisible = false">取消</button>
            <button type="submit" class="primary">保存</button>
          </div>
        </form>
      </div>
    </div>
  </div>
</template>

<script setup>
import { ref, reactive, onMounted } from 'vue'
import * as api from '../api/index.js'

const books = ref([])
const keyword = ref('')
const dialogVisible = ref(false)
const isEdit = ref(false)
const form = reactive({
  book_id: 0,
  isbn: '',
  title: '',
  author: '',
  publisher: '',
  category_id: '',
  total_stock: 0,
  location: '',
})

async function fetchBooks() {
  try {
    const res = await api.getBooks(keyword.value)
    books.value = res.data?.books || []
  } catch (e) {
    alert('加载失败：' + e.message)
  }
}

function openAddDialog() {
  isEdit.value = false
  Object.assign(form, { book_id: 0, isbn: '', title: '', author: '', publisher: '', category_id: '', total_stock: 0, location: '' })
  dialogVisible.value = true
}

function openEditDialog(book) {
  isEdit.value = true
  Object.assign(form, book)
  dialogVisible.value = true
}

async function submitBook() {
  try {
    if (isEdit.value) {
      await api.editBook(form.book_id, form)
    } else {
      await api.addBook(form)
    }
    dialogVisible.value = false
    await fetchBooks()
  } catch (e) {
    alert('操作失败：' + e.message)
  }
}

async function deleteBook(id) {
  if (!confirm('确认删除该书？')) return
  try {
    await api.deleteBook(id)
    await fetchBooks()
  } catch (e) {
    alert('删除失败：' + e.message)
  }
}

async function adjustStock(id, delta) {
  const qty = Math.abs(delta)
  try {
    if (delta > 0) {
      await api.increaseStock(id, qty)
    } else {
      await api.decreaseStock(id, qty)
    }
    await fetchBooks()
  } catch (e) {
    alert('调整库存失败：' + e.message)
  }
}

onMounted(fetchBooks)
</script>

<style scoped>
.book-manage { padding: 20px; }
.toolbar { display: flex; justify-content: space-between; margin-bottom: 20px; }
.search { display: flex; gap: 8px; }
.book-table { width: 100%; border-collapse: collapse; font-size: 14px; }
.book-table th, .book-table td { border: 1px solid #ebeef5; padding: 8px 10px; text-align: left; }
.book-table th { background: #f5f7fa; }
.book-table button { margin-right: 4px; }
.primary { background: #409eff; color: #fff; border: none; padding: 6px 16px; border-radius: 4px; cursor: pointer; }
.dialog-mask { position: fixed; inset: 0; background: rgba(0,0,0,0.35); display: flex; justify-content: center; align-items: center; z-index: 1000; }
.dialog-box { width: 500px; max-height: 80vh; overflow-y: auto; background: #fff; border-radius: 12px; padding: 24px; }
.form-group { margin-bottom: 14px; }
.form-group label { display: block; margin-bottom: 4px; font-weight: 500; }
.form-group input { width: 100%; padding: 8px 12px; border: 1px solid #dcdfe6; border-radius: 4px; }
.dialog-actions { display: flex; justify-content: flex-end; gap: 12px; margin-top: 16px; }
</style>

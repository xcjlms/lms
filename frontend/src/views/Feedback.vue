<template>
  <div class="feedback-submit">
    <h2>意见反馈</h2>
    <form @submit.prevent="submit">
      <div class="form-group">
        <label>标题 *</label>
        <input v-model="title" required />
      </div>
      <div class="form-group">
        <label>内容 *</label>
        <textarea v-model="content" rows="5" required></textarea>
      </div>
      <button type="submit" class="primary" :disabled="loading">提交反馈</button>
    </form>
    <div v-if="message" class="message" :class="messageType">{{ message }}</div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import * as api from '../api/index.js'

const raw = localStorage.getItem('lms_user')
const userInfo = raw ? JSON.parse(raw) : {}
const userId = userInfo.user_id || 0

const title = ref('')
const content = ref('')
const loading = ref(false)
const message = ref('')
const messageType = ref('info')

async function submit() {
  if (!userId) {
    message.value = '请先登录'
    messageType.value = 'error'
    return
  }
  if (!title.value.trim() || !content.value.trim()) return
  loading.value = true
  message.value = ''
  try {
    await api.submitFeedback(userId, title.value, content.value)
    message.value = '反馈提交成功，感谢您的建议！'
    messageType.value = 'success'
    title.value = ''
    content.value = ''
  } catch (e) {
    message.value = '提交失败：' + e.message
    messageType.value = 'error'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.feedback-submit { padding: 20px; max-width: 600px; margin: 0 auto; }
.form-group { margin-bottom: 16px; }
.form-group label { display: block; margin-bottom: 4px; font-weight: 500; }
.form-group input, .form-group textarea { width: 100%; padding: 8px 12px; border: 1px solid #dcdfe6; border-radius: 4px; font-family: inherit; }
.primary { background: #409eff; color: #fff; border: none; padding: 10px 24px; border-radius: 4px; cursor: pointer; }
.primary:disabled { opacity: 0.6; }
.message { margin-top: 16px; padding: 12px 16px; border-radius: 4px; }
.message.success { background: #f0f9eb; color: #67c23a; }
.message.error { background: #fef0f0; color: #f56c6c; }
</style>

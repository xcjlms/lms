<template>
  <div class="profile">
    <h2>个人信息修改</h2>
    <form @submit.prevent="changePwd">
      <div class="form-group">
        <label>当前密码 *</label>
        <input v-model="oldPwd" type="password" required />
      </div>
      <div class="form-group">
        <label>新密码 *</label>
        <input v-model="newPwd" type="password" required minlength="6" />
      </div>
      <div class="form-group">
        <label>确认新密码 *</label>
        <input v-model="confirmPwd" type="password" required />
      </div>
      <button type="submit" class="primary" :disabled="loading">修改密码</button>
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

const oldPwd = ref('')
const newPwd = ref('')
const confirmPwd = ref('')
const loading = ref(false)
const message = ref('')
const messageType = ref('info')

async function changePwd() {
  if (!userId) {
    message.value = '请先登录'
    messageType.value = 'error'
    return
  }
  if (newPwd.value !== confirmPwd.value) {
    message.value = '两次密码输入不一致'
    messageType.value = 'error'
    return
  }
  if (newPwd.value.length < 6) {
    message.value = '新密码至少6位'
    messageType.value = 'error'
    return
  }
  loading.value = true
  message.value = ''
  try {
    await api.changePassword(userId, oldPwd.value, newPwd.value)
    message.value = '密码修改成功！请重新登录。'
    messageType.value = 'success'
    // 可选：清除本地存储跳转到登录
    setTimeout(() => {
      localStorage.removeItem('lms_user')
      window.location.href = '/login'
    }, 2000)
  } catch (e) {
    message.value = '修改失败：' + e.message
    messageType.value = 'error'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.profile { padding: 20px; max-width: 500px; margin: 0 auto; }
.form-group { margin-bottom: 16px; }
.form-group label { display: block; margin-bottom: 4px; font-weight: 500; }
.form-group input { width: 100%; padding: 8px 12px; border: 1px solid #dcdfe6; border-radius: 4px; }
.primary { background: #409eff; color: #fff; border: none; padding: 10px 24px; border-radius: 4px; cursor: pointer; }
.primary:disabled { opacity: 0.6; }
.message { margin-top: 16px; padding: 12px 16px; border-radius: 4px; }
.message.success { background: #f0f9eb; color: #67c23a; }
.message.error { background: #fef0f0; color: #f56c6c; }
</style>

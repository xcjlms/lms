<template>
  <div class="backup-page">
    <h2>系统备份与恢复</h2>
    <div class="actions">
      <button @click="doBackup" :disabled="loading">创建备份</button>
      <div class="restore-group">
        <input v-model="restorePath" placeholder="输入备份文件名" />
        <button @click="doRestore" :disabled="loading || !restorePath">恢复备份</button>
      </div>
    </div>
    <div v-if="message" class="message" :class="messageType">{{ message }}</div>
  </div>
</template>

<script setup>
import { ref } from 'vue'
import * as api from '../api/index.js'

const loading = ref(false)
const message = ref('')
const messageType = ref('info')
const restorePath = ref('')

async function doBackup() {
  loading.value = true
  message.value = ''
  try {
    const res = await api.backupData()
    message.value = `备份成功：${res.data?.backup_path || '未知文件'}`
    messageType.value = 'success'
  } catch (e) {
    message.value = '备份失败：' + e.message
    messageType.value = 'error'
  } finally {
    loading.value = false
  }
}

async function doRestore() {
  if (!restorePath.value) return
  if (!confirm(`确认从 ${restorePath.value} 恢复数据库？当前数据将被覆盖。`)) return
  loading.value = true
  message.value = ''
  try {
    await api.restoreData(restorePath.value)
    message.value = '恢复成功！'
    messageType.value = 'success'
    restorePath.value = ''
  } catch (e) {
    message.value = '恢复失败：' + e.message
    messageType.value = 'error'
  } finally {
    loading.value = false
  }
}
</script>

<style scoped>
.backup-page { padding: 20px; }
.actions { display: flex; gap: 20px; align-items: center; flex-wrap: wrap; margin-bottom: 20px; }
.restore-group { display: flex; gap: 8px; align-items: center; }
.restore-group input { padding: 6px 10px; border: 1px solid #dcdfe6; border-radius: 4px; width: 240px; }
button { padding: 8px 20px; background: #409eff; color: #fff; border: none; border-radius: 4px; cursor: pointer; }
button:disabled { opacity: 0.6; cursor: not-allowed; }
.message { padding: 12px 16px; border-radius: 4px; }
.message.success { background: #f0f9eb; color: #67c23a; }
.message.error { background: #fef0f0; color: #f56c6c; }
</style>

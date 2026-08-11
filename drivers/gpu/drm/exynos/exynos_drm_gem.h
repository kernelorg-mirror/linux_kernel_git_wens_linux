/* SPDX-License-Identifier: GPL-2.0-or-later */
/* exynos_drm_gem.h
 *
 * Copyright (c) 2011 Samsung Electronics Co., Ltd.
 * Authoer: Inki Dae <inki.dae@samsung.com>
 */

#ifndef _EXYNOS_DRM_GEM_H_
#define _EXYNOS_DRM_GEM_H_

#include <drm/drm_gem.h>
#include <drm/drm_gem_dma_helper.h>
#include <linux/mm_types.h>

#define to_exynos_gem(x)	container_of(to_drm_gem_dma_obj(x), struct exynos_drm_gem, base)

#define IS_NONCONTIG_BUFFER(f)		(f & EXYNOS_BO_NONCONTIG)

/*
 * exynos drm buffer structure.
 *
 * @base: base GEM DMA object.
 * @flags: indicate memory type to allocated buffer and cache attruibute.
 *
 * P.S. this object would be transferred to user as kms_bo.handle so
 *	user can access the buffer through kms_bo.handle.
 */
struct exynos_drm_gem {
	struct drm_gem_dma_object	base;
	unsigned int			flags;
};

/* create a new buffer with gem object */
struct exynos_drm_gem *exynos_drm_gem_create(struct drm_device *dev,
					     unsigned int flags,
					     unsigned long size,
					     bool kvmap);

/*
 * request gem object creation and buffer allocation as the size
 * that it is calculated with framebuffer information such as width,
 * height and bpp.
 */
int exynos_drm_gem_create_ioctl(struct drm_device *dev, void *data,
				struct drm_file *file_priv);

/* get fake-offset of gem object that can be used with mmap. */
int exynos_drm_gem_map_ioctl(struct drm_device *dev, void *data,
			     struct drm_file *file_priv);

/*
 * get exynos drm object from gem handle, this function could be used for
 * other drivers such as 2d/3d acceleration drivers.
 * with this function call, gem object reference count would be increased.
 */
struct exynos_drm_gem *exynos_drm_gem_get(struct drm_file *filp,
					  unsigned int gem_handle);

/*
 * put exynos drm object acquired from exynos_drm_gem_get(),
 * gem object reference count would be decreased.
 */
static inline void exynos_drm_gem_put(struct exynos_drm_gem *exynos_gem)
{
	drm_gem_object_put(&exynos_gem->base.base);
}

/* get buffer information to memory region allocated by gem. */
int exynos_drm_gem_get_ioctl(struct drm_device *dev, void *data,
				      struct drm_file *file_priv);

/* create memory region for drm framebuffer. */
int exynos_drm_gem_dumb_create(struct drm_file *file_priv,
			       struct drm_device *dev,
			       struct drm_mode_create_dumb *args);

/* low-level interface prime helpers */
struct drm_gem_object *
exynos_drm_gem_prime_import_sg_table(struct drm_device *dev,
				     struct dma_buf_attachment *attach,
				     struct sg_table *sgt);

#endif

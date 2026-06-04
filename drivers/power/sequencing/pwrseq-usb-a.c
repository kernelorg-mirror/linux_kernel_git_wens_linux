// SPDX-License-Identifier: GPL-2.0-only
/*
 * Copyright (c) 2026 Google LLC
 * Author: Chen-Yu Tsai <wenst@chromium.org>
 *
 * Based on pwrseq-pcie-m2.c
 *
 * Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
 * Author: Manivannan Sadhasivam <manivannan.sadhasivam@oss.qualcomm.com>
 */

#include <linux/cleanup.h>
#include <linux/dev_printk.h>
#include <linux/device.h>
#include <linux/device/devres.h>
#include <linux/mod_devicetable.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_graph.h>
#include <linux/platform_device.h>
#include <linux/pwrseq/provider.h>
#include <linux/regulator/consumer.h>

struct pwrseq_usb_a_ctx {
	struct pwrseq_device *pwrseq;
	struct device_node *of_node;
	struct regulator *vbus;
	struct device *dev;
};

static int pwrseq_usb_a_vbus_enable(struct pwrseq_device *pwrseq)
{
	struct pwrseq_usb_a_ctx *ctx = pwrseq_device_get_drvdata(pwrseq);

	return regulator_enable(ctx->vbus);
}

static int pwrseq_usb_a_vbus_disable(struct pwrseq_device *pwrseq)
{
	struct pwrseq_usb_a_ctx *ctx = pwrseq_device_get_drvdata(pwrseq);

	return regulator_disable(ctx->vbus);
}

static const struct pwrseq_unit_data pwrseq_usb_a_unit_data = {
	.name = "vbus-enable",
	.enable = pwrseq_usb_a_vbus_enable,
	.disable = pwrseq_usb_a_vbus_disable,
};

static const struct pwrseq_target_data pwrseq_usb_a_target_data = {
	.name = "usb",
	.unit = &pwrseq_usb_a_unit_data,
};

static const struct pwrseq_target_data *pwrseq_usb_a_targets[] = {
	&pwrseq_usb_a_target_data,
	NULL
};

static int pwrseq_usb_a_match(struct pwrseq_device *pwrseq, struct device *dev)
{
	struct pwrseq_usb_a_ctx *ctx = pwrseq_device_get_drvdata(pwrseq);
	struct device_node *endpoint __free(device_node) = NULL;

	/*
	 * Traverse the 'remote-endpoint' nodes and check if the remote port
	 * matches the OF node of 'dev'.
	 */
	for_each_endpoint_of_node(ctx->of_node, endpoint) {
		struct device_node *remote_port __free(device_node) =
				of_graph_get_remote_port(endpoint);

		dev_info(dev, "match from %pOF to %pOF\n", endpoint, remote_port);
		if (!remote_port)
			continue;
		if (remote_port == dev_of_node(dev))
			return PWRSEQ_MATCH_OK;
	}

	return PWRSEQ_NO_MATCH;
}

static int pwrseq_usb_a_probe(struct platform_device *pdev)
{
	struct device *dev = &pdev->dev;
	struct pwrseq_usb_a_ctx *ctx;
	struct pwrseq_config config = {};

	ctx = devm_kzalloc(dev, sizeof(*ctx), GFP_KERNEL);
	if (!ctx)
		return -ENOMEM;

	platform_set_drvdata(pdev, ctx);
	ctx->of_node = dev_of_node(dev);

	ctx->vbus = devm_regulator_get(dev, "vbus");
	if (IS_ERR(ctx->vbus))
		return dev_err_probe(dev, PTR_ERR(ctx->vbus), "Failed to get VBUS supply\n");

	config.parent = dev;
	config.owner = THIS_MODULE;
	config.drvdata = ctx;
	config.match = pwrseq_usb_a_match;
	config.targets = pwrseq_usb_a_targets;

	ctx->pwrseq = devm_pwrseq_device_register(dev, &config);
	if (IS_ERR(ctx->pwrseq))
		return dev_err_probe(dev, PTR_ERR(ctx->pwrseq),
				     "Failed to register the power sequencer\n");

	return 0;
}

static const struct of_device_id pwrseq_usb_a_of_match[] = {
	{
		.compatible = "usb-a-connector",
	},
	{ }
};
MODULE_DEVICE_TABLE(of, pwrseq_usb_a_of_match);

static struct platform_driver pwrseq_usb_a_driver = {
	.driver = {
		.name = "pwrseq-usb-a",
		.of_match_table = pwrseq_usb_a_of_match,
	},
	.probe = pwrseq_usb_a_probe,
};
module_platform_driver(pwrseq_usb_a_driver);

MODULE_AUTHOR("Chen-Yu Tsai <wenst@chromium.org>");
MODULE_DESCRIPTION("Power Sequencing driver for USB A connector");
MODULE_LICENSE("GPL");
